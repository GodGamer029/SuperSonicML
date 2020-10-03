#pragma once

#include <bakkesmod/plugin/bakkesmodplugin.h>

#include <fstream>
#include <iostream>
#include <mutex>

#include "Constants.h"

namespace SuperSonicML::Share {
	extern std::shared_ptr<CVarManagerWrapper> cvarManager;
	extern std::shared_ptr<GameWrapper> gameWrapper;

	extern std::shared_ptr<bool> cvarEnabled;
	extern std::shared_ptr<bool> cvarEnableTraining;
	extern std::shared_ptr<bool> cvarEnableUserAsTeacher;
	extern std::shared_ptr<int> cvarBatchSize;
}