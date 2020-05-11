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
	auto ball = input.ball;
	auto car = input.car;

	if(!car.hasWheelContact){
		output.Throttle = 1;
		output.ActivateBoost = 0;
		output.HoldingBoost = 0;
		return;
	}

	{
		ControllerInput teacherOutput = {0};
		this->teacherBot->process(input, teacherOutput);

		if(!*SuperSonicML::Share::cvarEnableSlide)
			teacherOutput.Handbrake = 0;

		std::array<float, OUTPUT_SIZE> expectedOutputArray = {(float) teacherOutput.Throttle, (float) teacherOutput.Steer , teacherOutput.ActivateBoost ? 1.f : 0.f, teacherOutput.Handbrake ? 1 : 0};
		auto expectedOutput = torch::tensor(torch::ArrayRef(expectedOutputArray));

		// Make state for learner
		auto carPosAbs = car.pos;
		auto carAng = car.ang;
		auto ballPosAbs = ball.pos;
		auto ballVel = ball.vel;
		auto carForward = car.forward();
		auto carUp = car.up();
		auto targetLocal = dot(ball.pos - car.pos, car.orientation);
		auto forwardSpeed = dot(car.vel, car.forward());

		static auto normalizePosition = [](vec3c pos) {
		  vec3c normed = {pos[0] / 4096, pos[1] / 6000, pos[2] / 2100};
		  return clip(normed, -1, 1);
		};

		carPosAbs = normalizePosition(carPosAbs);
		ballPosAbs = normalizePosition(ballPosAbs);
		ballVel /= 4000;
		ballVel /= fmaxf(1.0f, norm(ballVel) / 1);

		targetLocal /= 2000;
		targetLocal /= fmaxf(1.0f, norm(targetLocal) / 1);

		carAng /= 5.5f;
		carAng /= fmaxf(1.0f, norm(carAng) / 1);

		forwardSpeed /= 2300;
		forwardSpeed = clip(forwardSpeed, -1, 1);

		std::array<float, INPUT_SIZE> netInputArray{ 0 };
		int counter = 0;
		static auto addVecToInputVec = [&](vec3c toAdd) {
		  netInputArray[counter++] = toAdd[0];
		  netInputArray[counter++] = toAdd[1];
		  netInputArray[counter++] = toAdd[2];
		};
		addVecToInputVec(carPosAbs);
		addVecToInputVec(carAng);
		addVecToInputVec(ballPosAbs);
		addVecToInputVec(ballVel);
		addVecToInputVec(carForward);
		addVecToInputVec(carUp);

		netInputArray[counter++] = forwardSpeed;

		if(counter != INPUT_SIZE){
			SuperSonicML::Share::cvarManager->log("INPUT_SIZE != counter "+std::to_string(INPUT_SIZE)+" != "+std::to_string(counter));
			return;
		}

		auto networkInput = torch::tensor(torch::ArrayRef(netInputArray));
		this->network->train(false);
		auto networkOutput = this->network->forward(networkInput);
		auto loss = torch::nn::functional::mse_loss(networkOutput, expectedOutput);
		float lossF = loss.item().toFloat();

		output.Throttle = clip(networkOutput[0].item().toFloat(), -1.f, 1.f);
		output.Steer = clip(networkOutput[1].item().toFloat(), -1.f, 1.f);
		output.ActivateBoost = networkOutput[2].item().toFloat() > 0.5f ? 1 : 0;
		output.HoldingBoost = output.ActivateBoost;
		output.Handbrake = networkOutput[3].item().toFloat() > 0.5f ? 1 : 0;

		static float avgLoss = 1; // Running average
		avgLoss = (avgLoss * 120 * 0.5f + lossF) / (120 * 0.5f + 1);
		if(car.carWrapper.GetPhysicsFrame() % 30 == 0)
			SuperSonicML::Share::cvarManager->log(std::string("Avg loss: ")+std::to_string(avgLoss)+" rep:"+std::to_string(this->totalLossInReplayMemory / fmax(1, this->replayMemory.size()))+" steer: "+std::to_string(networkOutput[1].item().toFloat())+" slide: "+std::to_string(networkOutput[3].item().toFloat())+" expected slide: "+std::to_string(teacherOutput.Handbrake));

		// add to replay memory
		{
			this->replayMemory.emplace_back(lossF, netInputArray, expectedOutputArray);
			this->totalLossInReplayMemory += lossF;

			if(this->replayMemory.size() > REPLAYMEMORY_MAX){
				auto toRemove = this->replayMemory.front();
				this->totalLossInReplayMemory -= std::get<0>(toRemove);
				this->replayMemory.pop_front();
			}
		}
	}

	// Train on replay memory
	if(this->replayMemory.size() > REPLAYMEMORY_MIN && this->totalLossInReplayMemory > 0.01f){
		static torch::optim::Adam optimizer(this->network->parameters(), torch::optim::AdamOptions(0.0005f /*learning rate*/));

		static std::random_device rd;
		static std::mt19937 e2(rd());
		constexpr float oscillationPreventer = 0.002f; // because this function prefers samples with high loss at the time they were added, we will hit a lot of samples that would already have a decreased loss in the current network, but are still being trained on which causes a lot of overfitting on these samples
		const auto batchSize = *SuperSonicML::Share::cvarBatchSize;
		for(int batchI = 0; batchI < batchSize; batchI++){
			std::uniform_real_distribution<> distribution(0, this->totalLossInReplayMemory * 0.99f + oscillationPreventer * this->replayMemory.size());
			double chosenOne = distribution(e2);
			int ind = -1;
			for(int i = 0; i < this->replayMemory.size(); i++) {
				auto tup = this->replayMemory[i];
				chosenOne -= std::get<0>(tup);
				chosenOne -= oscillationPreventer;
				if(chosenOne <= 0){
					ind = i;
					break;
				}
			}
			if(ind == -1){
				// Our total loss is off by a large amount, recalculate
				double orig = this->totalLossInReplayMemory;
				this->totalLossInReplayMemory = 0;
				for(auto ent : this->replayMemory) {
					this->totalLossInReplayMemory += std::get<0>(ent);
				}

				SuperSonicML::Share::cvarManager->log(std::string("total loss is off: orig=")+std::to_string(orig)+" now="+std::to_string(totalLossInReplayMemory));
				continue;
			}
			auto& chosen = this->replayMemory[ind];

			optimizer.zero_grad();
			auto networkInput = torch::tensor(torch::ArrayRef(std::get<1>(chosen)));
			auto expectedOutput =  torch::tensor(torch::ArrayRef(std::get<2>(chosen)));
			this->network->train();
			auto networkOutput = this->network->forward(networkInput);
			auto loss = torch::nn::functional::mse_loss(networkOutput, expectedOutput);
			auto lossFloat = loss.item().toFloat();
			// Update loss
			this->totalLossInReplayMemory -= std::get<0>(chosen);
			this->totalLossInReplayMemory += lossFloat;
			std::get<0>(chosen) = lossFloat;

			loss.backward();
			optimizer.step();
		}
	}
}
void TeacherLearnerExperiment::processAirBased(const BotInputData&, ControllerInput& output) {

}
