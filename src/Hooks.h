#pragma once

#include <bakkesmod/wrappers/GameObject/CarWrapper.h>
#include "bots/AerialAtbaBot.h"

struct ACar_TA_eventSetVehicleInput_Parms {
	struct ControllerInput NewInput;
	struct ControllerInput OldInput;
};

namespace SuperSonicML::Hooks {
	extern AerialAtbaBot atbaBot;

	void UpdateData(CarWrapper cw, void* params, std::string eventName);
}