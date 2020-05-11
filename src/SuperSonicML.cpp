#include "SuperSonicML.h"

namespace SuperSonicML::Plugin {

	BAKKESMOD_PLUGIN(SuperSonicMLPlugin, SuperSonicML::Constants::pluginName, SuperSonicML::Constants::versionString, 0)

	void SuperSonicMLPlugin::onLoad() {
		pluginInstance = this;

		SuperSonicML::Share::cvarManager = this->cvarManager;
		SuperSonicML::Share::gameWrapper = this->gameWrapper;

		this->cvarManager->registerCvar("supersonicml_enabled", "0", "Enable/Disable the plugin", true, true, 0.f, true, 1.f).bindTo(SuperSonicML::Share::cvarEnabled);
		if(this->gameWrapper->IsInFreeplay()) // Hot reloading
			this->cvarManager->executeCommand("supersonicml_enabled 1", true);
		this->cvarManager->registerCvar("supersonicml_slide_enabled", "0", "Enable/Disable slide", true, true, 0.f, true, 1.f).bindTo(SuperSonicML::Share::cvarEnableSlide);
		this->cvarManager->registerCvar("supersonicml_batch_size", "2", "", true, true, 1, true, 16).bindTo(SuperSonicML::Share::cvarBatchSize);


		this->gameWrapper->HookEventWithCaller<CarWrapper>("Function TAGame.Car_TA.SetVehicleInput", std::bind(&SuperSonicML::Hooks::UpdateData, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}

	void SuperSonicMLPlugin::onUnload() {
		pluginInstance = nullptr;
	}
}