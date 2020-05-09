#pragma once

#include "BotClass.h"

class AerialAtbaBot : BotClass {
public:
private:
	void process(const BotInputData& data) override;
	std::string getName() override;
};
