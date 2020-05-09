#pragma once

#include "BotClass.h"

class AtbaBot : BotClass {
public:
private:
	void process(const BotInputData& data) override;
	std::string getName() override;
};
