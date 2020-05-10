#include "AtbaBot.h"

void AtbaBot::process(const BotInputData& data, ControllerInput& output) {
	auto ball = data.ball;
	auto car = data.car;
	auto gravity = data.gravity;

	auto T = fmin(0.5, norm(vec2c(car.pos) - vec2c(ball.pos)) / (50 + norm(car.vel)));
	auto futureBallPos = ball.pos/* + ball.vel * T + 0.5 * gravity * T * T*/;
	futureBallPos[2] = clip(futureBallPos[2], 95, 2000);

	if(fabs(futureBallPos[1]) > 5120){ // Target inside goal
		if(fabs(car.pos[1]) < 5120 && fabs(car.pos[0]) > 750){ // Car outside goal
			futureBallPos[1] = 5100 * sgn(futureBallPos[1]);
		}
	}

	if(fabs(car.pos[1]) > 5120){ // stuck in goal
		futureBallPos = { 0, 0, 0 };
	}

	if(futureBallPos[2] > 100 && norm(vec2c(car.pos) - vec2c(futureBallPos)) > 500){ // Target in air
		futureBallPos[2] = 0;
	}

	auto targetLocal = dot(futureBallPos - car.pos, car.orientation);
	auto angle = atan2(targetLocal[1], targetLocal[0]);
	output.Steer = clip(3 * angle, -1, 1);

	output.Throttle = clip(norm(vec2c(car.pos) - vec2c(futureBallPos)) / 100, 0.05, 1);

	if(fabs(angle) > 1 && dot(car.vel, car.forward()) > 800)
		output.Throttle = -1;

	if(targetLocal[0] > 1000 && fabs(angle) < 0.1f && dot(car.vel, car.forward()) < 1800)
		output.ActivateBoost = 1;
	else if(fabs(angle) > 0.9 && fabs(dot(car.ang, car.orientation)[2]) < 3 && dot(car.vel, car.forward()) > 300)
		output.Handbrake = 1;
}

std::string AtbaBot::getName() {
	return "Atba";
}
