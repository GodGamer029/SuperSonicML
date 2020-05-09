#include "AerialAtbaBot.h"

void AerialAtbaBot::process(const BotInputData& data, ControllerInput& output) {

	auto gravity = data.gravity;
	auto ball = data.ball;
	auto car = data.car;

	auto targetPos = ball.pos;

	auto dist = norm(car.pos - targetPos);
	auto timeToArrival = clip(dist / (float) fmax(700, 100 + norm(car.vel)), 0.01f, 4.f);

	vec3c xf = car.pos +
			   car.vel * timeToArrival +
			   0.5 * gravity * timeToArrival * timeToArrival;

	targetPos += ball.vel * timeToArrival;
	targetPos[2] = clip(targetPos[2], 95, 5000);

	vec3c deltaX = targetPos - xf;
	vec3c direction = normalize(deltaX);

	// Aerial pd
	if (!car.hasWheelContact) {
		auto carOrientation = car.orientation;
		auto carForward = vec3c{carOrientation(0, 0), carOrientation(1, 0), carOrientation(2, 0)};

		auto localAng = dot(car.ang, carOrientation);
		auto localTarget = dot(direction, carOrientation);

		// "borrowed" aerial controller from DaCoolOne's tutorial
		auto yawAngle = atan2(-localTarget[1], localTarget[0]);
		auto pitchAngle = atan2(-localTarget[2], localTarget[0]);

		auto P = 5.f;
		auto D = 0.8f;

		auto yaw = yawAngle * -P - localAng[2] * D;
		auto pitch = localAng[1] * D + pitchAngle * -P;

		output.Pitch = clip(pitch, -1, 1);
		output.Yaw = clip(yaw, -1, 1);

		if (dot(carForward, direction) > 0.6f && norm(deltaX) > 20) {
			output.HoldingBoost = 1;
			output.ActivateBoost = 1;
		}

		if (dot(carForward, direction) > 0.8f) {
			output.Roll = (dot(carForward, direction) - 0.8f) * 5;
		}

	} else if (car.carWrapper.GetbJumped() == 0 || car.carWrapper.GetJumpComponent().CanActivate()) {
		if (rand() % 5 == 0 && norm(car.ang) < 0.3f)
			output.Jump = 1;
	}
	output.Throttle = 0.05f; // Helps to flip the car back up onto the wheels
}

std::string AerialAtbaBot::getName() {
	return "AerialAtba";
}
