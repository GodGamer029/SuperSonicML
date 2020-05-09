#pragma once

#include "BotClass.h"

class AtbaBot : BotClass {
public:
	void process(const BotInputData& data, ControllerInput& output) override;
	std::string getName() override;
};
