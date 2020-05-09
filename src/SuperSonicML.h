#pragma once

#pragma comment(lib, "bakkesmod.lib")
#include <bakkesmod/plugin/bakkesmodplugin.h>

#include <memory>

#include "Constants.h"
#include "GameData.h"
#include "Hooks.h"

namespace SuperSonicML::Plugin {

	class SuperSonicMLPlugin : public BakkesMod::Plugin::BakkesModPlugin {
	public:
		virtual void onLoad() override;
		virtual void onUnload() override;
	};

	SuperSonicMLPlugin* pluginInstance = nullptr;
}
