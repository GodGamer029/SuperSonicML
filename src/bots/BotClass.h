#pragma once

#include "math/math.h"

struct BotInputData {
	struct {
		const vec3c pos, vel, ang;
	} const ball;
	struct {
		const vec3c pos, vel, ang;
		const mat3 orientation;
	} const car;
};

class BotClass {
public:
	virtual void process(const BotInputData&) = 0;
	virtual std::string getName() { return "PureBot"; }
};
