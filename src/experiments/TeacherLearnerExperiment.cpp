#include "TeacherLearnerExperiment.h"

#include <filesystem>
#include <sstream>

//template<typename T>
TeacherLearnerExperiment /*<T, EnableIfBot<T>>*/ ::TeacherLearnerExperiment(std::shared_ptr<AerialAtbaBot> bot) {
	this->teacherBot = bot;
	this->network = std::make_shared<Net>();

	//torch::serialize::InputArchive inArchive;
	//inArchive.load_from("model-25-06-2020_19-11-39.dat");
	//this->network->load(inArchive);
}

//template<typename T>
void TeacherLearnerExperiment /*<T, EnableIfBot<T>>*/ ::process(const BotInputData& input, ControllerInput& output) {
	this->processGroundBased(input, output);
}
void TeacherLearnerExperiment::processGroundBased(const BotInputData& input, ControllerInput& output) {
	auto ball = input.ball;
	auto car = input.car;

	{
		ControllerInput teacherOutput = {};
		if(*SuperSonicML::Share::cvarEnableUserAsTeacher){
			// user is teacher
			teacherOutput = input.originalInput;
		}else{
			this->teacherBot->process(input, teacherOutput);
		}

		std::array<float, OUTPUT_SIZE> expectedOutputArray = {(float) teacherOutput.Throttle, (float) teacherOutput.Steer, teacherOutput.ActivateBoost ? 1.f : 0.f, teacherOutput.Handbrake ? 1.f : 0.f, teacherOutput.Pitch, teacherOutput.Yaw, teacherOutput.Roll, static_cast<float>(teacherOutput.Jump)};
		auto expectedOutput = torch::tensor(torch::ArrayRef(expectedOutputArray));

		// Make state for learner
		auto carPosAbs = car.pos;
		auto carAng = car.ang;
		auto carVel = car.vel;
		auto ballPosAbs = ball.pos;
		auto ballVel = ball.vel;
		auto carForward = car.forward();
		auto carUp = car.up();
		auto targetLocal = dot(ball.pos - car.pos, car.orientation);
		auto forwardSpeed = dot(car.vel, car.forward());
		auto isOnGround = car.hasWheelContact ? 1.f : 0.f;
		auto canDodge = !isOnGround && !car.carWrapper.GetDodgeComponent().IsNull() && car.carWrapper.GetDodgeComponent().CanActivate();

		static auto normalizePosition = [](vec3c pos) {
		  vec3c normed = {pos[0] / 4150, pos[1] / 6000, pos[2] / 2100};
		  return clip(normed, -1, 1);
		};

		carPosAbs = normalizePosition(carPosAbs);
		ballPosAbs = normalizePosition(ballPosAbs);
		ballVel /= 4000;
		ballVel /= fmaxf(1.0f, norm(ballVel) / 1);

		targetLocal /= 1500;
		targetLocal /= fmaxf(1.0f, norm(targetLocal) / 1);

		carAng /= 5.5f;
		carAng /= fmaxf(1.0f, norm(carAng) / 1);
		carVel /= 2300;
		carVel /= fmaxf(1.0f, norm(carVel) / 1);

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
		addVecToInputVec(carVel);
		addVecToInputVec(carForward);
		addVecToInputVec(carUp);
		addVecToInputVec(ballPosAbs);
		addVecToInputVec(ballVel);
		addVecToInputVec(targetLocal);

		netInputArray[counter++] = forwardSpeed;
		netInputArray[counter++] = isOnGround;
		netInputArray[counter++] = canDodge;

		if(counter != INPUT_SIZE){
			SuperSonicML::Share::cvarManager->log("INPUT_SIZE != counter "+std::to_string(INPUT_SIZE)+" != "+std::to_string(counter));
			return;
		}

		auto networkInput = torch::tensor(torch::ArrayRef(netInputArray));
		this->network->train(*SuperSonicML::Share::cvarEnableTraining);
		auto networkOutput = this->network->forward(networkInput);
		auto loss = torch::nn::functional::mse_loss(networkOutput, expectedOutput);
		float multiplier = 1;

		//if(!car.hasWheelContact)
		//	multiplier = 0;

		// Close encounters with the ball are important to learn
		if(norm(car.pos - ball.pos) < 92 + 60 + 50)
			multiplier *= 1.2f;

		if(norm(car.vel) < 200) // basically standing still
			multiplier *= 1.4f;

		static bool lastJump = false;

		//if(teacherOutput.Jump == 1ui32 && !lastJump) // rising edge
		//	multiplier *= 5;

		if(*SuperSonicML::Share::cvarEnableTraining && *SuperSonicML::Share::cvarEnableUserAsTeacher && (car.carWrapper.GetPhysicsFrame() % 12 < 8 || !car.hasWheelContact/*(teacherOutput.Jump == 1ui32 && !lastJump)*/)){
			output = teacherOutput;
		}else{
			output.Throttle = clip(networkOutput[0].item().toFloat(), -1.f, 1.f);
			output.Steer = clip(networkOutput[1].item().toFloat(), -1.f, 1.f);
			output.ActivateBoost = networkOutput[2].item().toFloat() > 0.5f ? 1 : 0;
			output.HoldingBoost = output.ActivateBoost;
			output.Handbrake = networkOutput[3].item().toFloat() > 0.5f ? 1 : 0;

			multiplier *= 0.4f;
			output.Pitch = clip(networkOutput[4].item().toFloat(), -1.f, 1.f);
			output.Yaw = clip(networkOutput[5].item().toFloat(), -1.f, 1.f);
			output.Roll = clip(networkOutput[6].item().toFloat(), -1.f, 1.f);
			if(*SuperSonicML::Share::cvarEnableTraining){
				output.Jump = teacherOutput.Jump;
			}else{
				output.Jump = networkOutput[7].item().toFloat() > 0.5f ? 1 : 0;
			}

		}
		lastJump = teacherOutput.Jump == 1ui32;

		loss = loss * multiplier;

		float lossF = loss.item().toFloat();

		static float avgLoss = 1; // Running average
		avgLoss = (avgLoss * 120 * 0.5f + lossF) / (120 * 0.5f + 1);
		if(car.carWrapper.GetPhysicsFrame() % 60 == 0)
			SuperSonicML::Share::cvarManager->log(std::string("avg_loss: ")+std::to_string(avgLoss)+" replay_avg_loss:"+std::to_string(this->totalLossInReplayMemory / fmax(1, this->replayMemory.size()))+" steer: "+std::to_string(networkOutput[1].item().toFloat())+" slide: "+std::to_string(networkOutput[3].item().toFloat()));

		// add to replay memory
		if(*SuperSonicML::Share::cvarEnableTraining){
			if(multiplier > 0){
				this->replayMemory.emplace_back(lossF, netInputArray, expectedOutputArray, multiplier);
				this->totalLossInReplayMemory += lossF;
			}

			if(this->replayMemory.size() > REPLAYMEMORY_MAX){
				auto toRemove = this->replayMemory.front();
				this->totalLossInReplayMemory -= std::get<0>(toRemove);
				this->replayMemory.pop_front();
			}
		}else if(!this->replayMemory.empty()){
			this->replayMemory.clear();
			this->totalLossInReplayMemory = 0;
		}
	}

	// Train on replay memory
	if(this->replayMemory.size() > REPLAYMEMORY_MIN && this->totalLossInReplayMemory > 0.01f && *SuperSonicML::Share::cvarEnableTraining){
		static torch::optim::Adam optimizer(this->network->parameters(), torch::optim::AdamOptions(0.00005f /*learning rate*/));

		static std::random_device rd;
		static std::mt19937 e2(rd());
		constexpr float oscillationPreventer = 0.005f; // because this function prefers samples with high loss at the time they were added, we will hit a lot of samples that would already have a decreased loss in the current network, but are still being trained on which causes a lot of overfitting on these samples
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
			auto multiplier = std::get<3>(chosen);
			loss = loss * multiplier;
			auto lossFloat = loss.item().toFloat();
			// Update loss
			this->totalLossInReplayMemory -= std::get<0>(chosen);
			this->totalLossInReplayMemory += lossFloat;
			std::get<0>(chosen) = lossFloat;

			loss.backward();
			optimizer.step();
		}

		// save model every 2 minutes
		static auto lastSave = std::chrono::system_clock::now();

		auto now = std::chrono::system_clock::now();
		auto elapsedSec = std::chrono::duration_cast<std::chrono::seconds>(now - lastSave);

		if (elapsedSec.count() >= 2 * 60) {
			std::time_t t = std::time(nullptr);
			std::tm tm = *std::localtime(&t);

			std::stringstream pathS;
			pathS << "model-" << std::put_time(&tm, "%d-%m-%Y_%H-%M-%S") << ".dat";
			auto pathAbs = std::filesystem::absolute(pathS.str());
			torch::serialize::OutputArchive output_archive;
			this->network->save(output_archive);
			output_archive.save_to(pathAbs.string());

			char buf[300];
			sprintf_s(buf, "Saving model after %i seconds to %s %s", (int) elapsedSec.count(), pathS.str().c_str(), pathAbs.string().c_str());
			SuperSonicML::Share::cvarManager->log(buf);
			lastSave = now;
		}
	}
}
void TeacherLearnerExperiment::processAirBased(const BotInputData&, ControllerInput& output) {

}
