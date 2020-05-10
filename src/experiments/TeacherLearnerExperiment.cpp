#include "TeacherLearnerExperiment.h"

//template<typename T>
TeacherLearnerExperiment /*<T, EnableIfBot<T>>*/ ::TeacherLearnerExperiment(std::shared_ptr<AtbaBot> bot) {
	this->teacherBot = bot;
	this->network = std::make_shared<Net>();
}

//template<typename T>
void TeacherLearnerExperiment /*<T, EnableIfBot<T>>*/ ::process(const BotInputData& input, ControllerInput& output) {
	this->processGroundBased(input, output);
}
void TeacherLearnerExperiment::processGroundBased(const BotInputData& input, ControllerInput& output) {
	ControllerInput teacherOutput;
	this->teacherBot->process(input, teacherOutput);
	std::vector<float> expectedOutputVec = {(float) teacherOutput.Throttle, (float) teacherOutput.Steer , teacherOutput.ActivateBoost ? 1.f : 0.f};
	auto expectedOutput = torch::tensor(expectedOutputVec);

	// Make state for learner
	auto ball = input.ball;
	auto car = input.car;

	if(!car.hasWheelContact){
		output.Throttle = 1;
		return;
	}


	auto carPosAbs = car.pos;
	auto ballPosAbs = ball.pos;
	auto carForward = car.forward();
	auto targetLocal = dot(ball.pos - car.pos, car.orientation);
	//auto dist = norm(ball.pos - car.pos);
	auto forwardSpeed = dot(car.vel, car.forward());

	static auto normalizePosition = [](vec3c pos) {
	  vec3c normed = {pos[0] / 4096, pos[1] / 6000 /*include goal*/, pos[2] / 2100};
	  //return clip(normed, -1, 1);
	  return normed;
	};

	carPosAbs = normalizePosition(carPosAbs);
	ballPosAbs = normalizePosition(ballPosAbs);

	// Normalize
	targetLocal /= 2000;
	targetLocal /= fmaxf(1.0f, norm(targetLocal) / 1);

	//dist /= 6000;
	//dist = clip(dist, -1, 1);

	forwardSpeed /= 2300;
	forwardSpeed = clip(forwardSpeed, -1, 1);

	static torch::optim::Adam optimizer(this->network->parameters(), torch::optim::AdamOptions(0.0002f /*learning rate*/));

	std::vector<float> netInputVector;
	netInputVector.reserve(4 * 3 + 2);
	static auto addVecToInputVec = [&](vec3c toAdd) {
	  netInputVector.push_back(toAdd[0]);
	  netInputVector.push_back(toAdd[1]);
	  netInputVector.push_back(toAdd[2]);
	};

	//addVecToInputVec(targetLocal);
	addVecToInputVec(carPosAbs);
	addVecToInputVec(ballPosAbs);
	addVecToInputVec(carForward);
	//netInputVector.push_back(dist);
	netInputVector.push_back(forwardSpeed);

	auto networkInput = torch::tensor(netInputVector);
	this->network->train(false);
	auto networkOutput = this->network->forward(networkInput);
	auto loss = torch::nn::functional::mse_loss(networkOutput, expectedOutput);
	float lossF = loss.item().toFloat();

	output.Throttle = clip(networkOutput[0].item().toFloat(), -1.f, 1.f);
	output.Steer = clip(networkOutput[1].item().toFloat(), -1.f, 1.f);
	output.ActivateBoost = networkOutput[2].item().toFloat() > 0.5f ? 1 : 0;

	if((car.carWrapper.GetPhysicsFrame() / 120) % 10 == 0)
		return;

	static float avgLoss = 1; // Running average
	avgLoss = (avgLoss * 120 * 0.5f + lossF) / (120 * 0.5f + 1);
	if(car.carWrapper.GetPhysicsFrame() % 30 == 0)
		SuperSonicML::Share::cvarManager->log(std::string("Avg loss: ")+std::to_string(avgLoss)+" rep:"+std::to_string(this->totalLossInReplayMemory / fmax(1, this->replayMemory.size()))+" steer: "+std::to_string(networkOutput[1].item().toFloat()));

	constexpr auto REPLAYMEMORY_MIN = 120 * 5;
	constexpr auto REPLAYMEMORY_MAX = 120 * 60 * 2;

	// add to replay memory
	{
		this->replayMemory.emplace_back(lossF, netInputVector, expectedOutputVec);
		this->totalLossInReplayMemory += lossF;

		/*if(lossF > avgLoss || this->replayMemory.size() <= 100){
			optimizer.zero_grad();
			loss.backward();
			optimizer.step();
		}*/

		if(this->replayMemory.size() > REPLAYMEMORY_MAX){
			auto toRemove = this->replayMemory.front();
			this->totalLossInReplayMemory -= std::get<0>(toRemove);

			this->replayMemory.pop_front();
		}
	}

	if(this->replayMemory.size() > REPLAYMEMORY_MIN && this->totalLossInReplayMemory > 0.01f){
		static std::random_device rd;
		static std::mt19937 e2(rd());
		constexpr float oscillationPreventer = 0.01f; // prevents oscilliation, because this function prefers samples with high loss at the time they were added, we will hit a lot of samples that would already have a decreased loss in the current network, but are still being trained on which causes a lot of overfitting on these samples
		for(int batchI = 0; batchI < 2; batchI++){
			std::uniform_real_distribution<> distribution(0, this->totalLossInReplayMemory * 0.99f + oscillationPreventer * this->replayMemory.size());
			double chosenOne = distribution(e2);
			int ind = -1;
			for(int i = 0; i < this->replayMemory.size(); i++) {
				auto tup = this->replayMemory[i];
				chosenOne -= std::get<0>(tup);
				chosenOne -= oscillationPreventer;
				if(chosenOne <= 0){
					// train on this one
					ind = i;
					break;
				}
			}
			if(ind == -1){
				// Our total loss is off by a large amount, recalculate
				float orig = this->totalLossInReplayMemory;
				this->totalLossInReplayMemory = 0;
				for(auto ent : this->replayMemory) {
					this->totalLossInReplayMemory += std::get<0>(ent);
				}

				SuperSonicML::Share::cvarManager->log(std::string("total loss is off: orig=")+std::to_string(orig)+" now="+std::to_string(totalLossInReplayMemory));
				continue;
			}
			auto& chosen = this->replayMemory[ind];

			optimizer.zero_grad();

			auto networkInputB = torch::tensor(std::get<1>(chosen));
			auto expectedOutputB =  torch::tensor(std::get<2>(chosen));
			this->network->train();
			auto networkOutputB = this->network->forward(networkInputB);
			auto lossB = torch::nn::functional::mse_loss(networkOutputB, expectedOutputB);
			auto lossBF = lossB.item().toFloat();
			this->totalLossInReplayMemory -= std::get<0>(chosen);
			this->totalLossInReplayMemory += lossBF;
			std::get<0>(chosen) = lossBF;

			lossB.backward();
			optimizer.step();
		}
	}
}
void TeacherLearnerExperiment::processAirBased(const BotInputData&, ControllerInput& output) {

}
