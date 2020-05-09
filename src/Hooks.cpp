#include "Hooks.h"

#include <torch/torch.h>

#include <iostream>
#include <memory>
#include <string>

#include "GameData.h"
#include "math/math.h"// Headers taken from RLU

namespace SuperSonicML::Hooks {

	AerialAtbaBot atbaBot;

	void UpdateData(CarWrapper myCar, void* pVoidParams, std::string eventName) {
		if (!*SuperSonicML::Share::cvar_enabled)
			return;

		if (SuperSonicML::Share::gameWrapper->IsInOnlineGame())
			return;

		static auto lastMsg = std::chrono::system_clock::now();
		static int ticks = 0;

		ticks++;
		// Some computation here
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

		auto* vehicleInput = reinterpret_cast<ACar_TA_eventSetVehicleInput_Parms*>(pVoidParams);
		auto controller = myCar.GetPlayerController();

		if (controller.IsNull())
			return;

		float oldFrameTime = SuperSonicML::Share::lastGameFrameTime;
		SuperSonicML::Share::lastGameFrameTime = myCar.GetPhysicsTime();
		float timeDelta = SuperSonicML::Share::lastGameFrameTime - oldFrameTime;

		auto gameEvent = controller.GetGameEvent();

		if (gameEvent.memory_address == 0)
			return;

		auto soccarEvent = ServerWrapper(gameEvent.memory_address);
		if (soccarEvent.IsNull())
			return;

		auto gameBalls = soccarEvent.GetGameBalls();
		auto allCars = soccarEvent.GetCars();

		if (gameBalls.Count() == 1 && gameBalls.Get(0).GetExplosionTime() <= 0 && allCars.Count() == 1 && soccarEvent.GetbRoundActive()) {
			auto ball = gameBalls.Get(0);

			mat3 carOrientation;
			{
				auto quat = myCar.GetRBState().Quaternion;

				carOrientation = quaternion_to_rotation(vec4c({-quat.W, -quat.X, -quat.Y, -quat.Z}));
			}
			vec3c targetPos = toVec3(ball.GetRBState().Location);
			float grav = myCar.GetGravityZ();
			//float timeToArrival = ceil(myCar.GetPhysicsTime() / 2) * 2 - myCar.GetPhysicsTime();
			auto dist = norm(toVec3(myCar.GetRBState().Location) - targetPos);
			float timeToArrival = clip(dist / fmax(700, 100 + norm(toVec3(myCar.GetRBState().LinearVelocity))),
									   0.01f, 4.f);
			vec3c xf = toVec3(myCar.GetRBState().Location) +
					   toVec3(myCar.GetRBState().LinearVelocity) * timeToArrival +
					   0.5 * vec3c{0, 0, grav} * timeToArrival * timeToArrival;

			targetPos += toVec3(ball.GetRBState().LinearVelocity) * timeToArrival;
			targetPos[2] = clip(targetPos[2], 95, 5000);

			vec3c delta_x = targetPos - xf;
			vec3c direction = normalize(delta_x);
			// Aerial pd
			if (myCar.GetNumWheelWorldContacts() < 2) {
				auto carForward = vec3c{carOrientation(0, 0), carOrientation(1, 0), carOrientation(2, 0)};
				auto ang = toVec3(myCar.GetRBState().AngularVelocity);

				auto localAng = dot(ang, carOrientation);
				auto localTarg = dot(direction, carOrientation);

				// "borrowed" aerial controller from DaCoolOne's tutorial
				auto yawAngle = atan2(-localTarg[1], localTarg[0]);
				auto pitchAngle = atan2(-localTarg[2], localTarg[0]);

				auto P = 5.f;
				auto D = 0.8f;

				auto yaw = yawAngle * -P - localAng[2] * D;
				auto pitch = localAng[1] * D + pitchAngle * -P;

				vehicleInput->NewInput.Pitch = clip(pitch, -1, 1);
				vehicleInput->NewInput.Yaw = clip(yaw, -1, 1);

				if (dot(carForward, direction) > 0.95 && norm(delta_x) > 20) {
					vehicleInput->NewInput.ActivateBoost = 1;
					vehicleInput->NewInput.HoldingBoost = 1;
				}

				/*
				if (dot(carForward, direction) > 0.8f) {
					vehicleInput->NewInput.Roll = (dot(carForward, direction) - 0.8f) * 5;
				}*/

			} else if (myCar.GetbJumped() == 0 || myCar.GetJumpComponent().CanActivate()) {
				if (rand() % 5 == 0)
					vehicleInput->NewInput.Jump = 1;
			}
			ball.SetCarBounceScale(3);
		}
	}
}