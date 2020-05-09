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

		this->gameWrapper->HookEventWithCaller<CarWrapper>("Function TAGame.Car_TA.SetVehicleInput", std::bind(&SuperSonicML::Hooks::UpdateData, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}

	void SuperSonicMLPlugin::onUnload() {
		pluginInstance = nullptr;
	}
}