#include "GameData.h"

namespace SuperSonicML::Share {
	std::shared_ptr<CVarManagerWrapper> cvarManager;
	std::shared_ptr<GameWrapper> gameWrapper;

	std::shared_ptr<bool> cvarEnabled = std::make_shared<bool>();
	std::shared_ptr<bool> cvarEnableTraining = std::make_shared<bool>();
	std::shared_ptr<bool> cvarEnableUserAsTeacher = std::make_shared<bool>();
	std::shared_ptr<int> cvarBatchSize = std::make_shared<int>(2);
}