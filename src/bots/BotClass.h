#pragma once

#include <bakkesmod/wrappers/GameObject/CarWrapper.h>

#include "math/math.h"

struct BotInputData {
public:
	struct BallData {
		const vec3c pos, vel, ang;
	} const ball;
	struct CarData {
		const vec3c pos, vel, ang;
		const mat3 orientation;
		const bool hasWheelContact;
		CarWrapper carWrapper;

		vec3c forward(){
			return {orientation(0, 0), orientation(1, 0), orientation(2, 0)};
		}

		vec3c right(){
			return {orientation(0, 1), orientation(1, 1), orientation(2, 1)};
		}

		vec3c up(){
			return {orientation(0, 2), orientation(1, 2), orientation(2, 2)};
		}
	} const car;
	const vec3c gravity;
	const float elapsedSeconds;
	const ControllerInput originalInput;
};

class BotClass {
public:
	virtual void process(const BotInputData&, ControllerInput& output) = 0;
	virtual std::string getName() { return "PureBot"; }
};
