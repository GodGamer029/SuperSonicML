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
	} const car;
	const vec3c gravity;
	const float elapsedSeconds;
};

class BotClass {
public:
	virtual void process(const BotInputData&, ControllerInput& output) = 0;
	virtual std::string getName() { return "PureBot"; }
};
