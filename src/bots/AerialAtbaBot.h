#pragma once

#include "BotClass.h"
#include <bakkesmod/wrappers/GameObject/CarComponent/JumpComponentWrapper.h>

class AerialAtbaBot : BotClass {
public:
	void process(const BotInputData& data, ControllerInput& output) override;
	std::string getName() override;
};
