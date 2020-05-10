#pragma once

#include <bakkesmod/wrappers/GameObject/CarWrapper.h>
#include <bots/AerialAtbaBot.h>
#include <bots/AtbaBot.h>
#include <experiments/TeacherLearnerExperiment.h>

struct ACar_TA_eventSetVehicleInput_Params {
	struct ControllerInput NewInput;
	struct ControllerInput OldInput;
};

namespace SuperSonicML::Hooks {
	extern AerialAtbaBot atbaBot;

	void UpdateData(CarWrapper cw, void* params, const std::string& eventName);
}