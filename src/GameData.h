#pragma once

#include <bakkesmod/plugin/bakkesmodplugin.h>

#include <fstream>
#include <iostream>
#include <mutex>

#include "Constants.h"

namespace SuperSonicML::Share {
	extern std::shared_ptr<CVarManagerWrapper> cvarManager;
	extern std::shared_ptr<GameWrapper> gameWrapper;

	extern std::shared_ptr<bool> cvar_enabled;

	extern int lastLocalScore;
	extern int lastEnemyScore;

	extern float lastGameFrameTime;
}