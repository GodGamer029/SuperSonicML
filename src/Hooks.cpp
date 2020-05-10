#include "Hooks.h"

#include <torch/torch.h>

#include <iostream>
#include <memory>
#include <string>

#include "GameData.h"
#include "math/math.h"// Headers taken from RLU

namespace SuperSonicML::Hooks {

	void UpdateData(CarWrapper myCar, void* pVoidParams, const std::string& eventName) {
		if (!*SuperSonicML::Share::cvarEnabled)
			return;

		if (SuperSonicML::Share::gameWrapper->IsInOnlineGame())
			return;

		static auto lastMsg = std::chrono::system_clock::now();
		static int ticks = 0;

		ticks++;

		auto now = std::chrono::system_clock::now();
		auto elapsedMilli = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastMsg);

		if (elapsedMilli.count() >= 1000) {
			char buf[200];
			torch::Tensor tensor = torch::eye(3);
			sprintf_s(buf, "Took: %d %d %s", ticks, (int) elapsedMilli.count(), tensor.toString().c_str());
			SuperSonicML::Share::cvarManager->log(buf);
			lastMsg = now;
			ticks = 0;
		}

		if (pVoidParams == nullptr)
			return;

		auto* vehicleInput = reinterpret_cast<ACar_TA_eventSetVehicleInput_Params*>(pVoidParams);
		auto controller = myCar.GetPlayerController();

		if (controller.IsNull())
			return;

		auto gameEvent = controller.GetGameEvent();

		if (gameEvent.memory_address == 0)
			return;

		auto serverWrapper = ServerWrapper(gameEvent.memory_address);
		if (serverWrapper.IsNull())
			return;

		auto gameBalls = serverWrapper.GetGameBalls();
		auto allCars = serverWrapper.GetCars();

		if (gameBalls.Count() == 1 && gameBalls.Get(0).GetExplosionTime() <= 0 && allCars.Count() == 1 && serverWrapper.GetbRoundActive()) {
			auto ball = gameBalls.Get(0);
			auto ballRbState = ball.GetRBState();

			auto ballData = BotInputData::BallData{ toVec3(ballRbState.Location), toVec3(ballRbState.LinearVelocity), toVec3(ballRbState.AngularVelocity) };
			auto gravity = vec3c { 0, 0, myCar.GetGravityZ() };

			auto carRbState = myCar.GetRBState();

			mat3 carOrientation;
			{
				auto quaternion = carRbState.Quaternion;
				carOrientation = quaternion_to_rotation(vec4c{-quaternion.W, -quaternion.X, -quaternion.Y, -quaternion.Z});
			}
			auto carData = BotInputData::CarData{ toVec3(carRbState.Location), toVec3(carRbState.LinearVelocity), toVec3(carRbState.AngularVelocity), carOrientation, myCar.GetNumWheelWorldContacts() >= 3, myCar };

			auto botInputData = BotInputData{ ballData, carData, gravity, myCar.GetPhysicsTime() };

			static auto currentExperiment = TeacherLearnerExperiment(std::make_shared<AtbaBot>());
			currentExperiment.process(botInputData, vehicleInput->NewInput);

			static std::once_flag onceFlag;

			std::call_once(onceFlag, [&](){
			  ball.SetCarBounceScale(2); //

			  ball.SetBallScale(2); // Make it easier to hit
			  ball.SetWorldBounceScale(2);
			  ball.SetBallGravityScale(1);

			  // more predictable
			  ball.SetMaxLinearSpeed(2000);
			  ball.SetMaxAngularSpeed(1);
			});

			auto alteredVel = ballData.vel;
			if(alteredVel[2] > 0)
				alteredVel[2] *= 0.95f; // Dampen z vel
			ball.SetVelocity(Vector(alteredVel[0], alteredVel[1], alteredVel[2]));
		}
	}
}