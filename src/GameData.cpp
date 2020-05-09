#include "GameData.h"

namespace SuperSonicML::Share {
	std::shared_ptr<CVarManagerWrapper> cvarManager;
	std::shared_ptr<GameWrapper> gameWrapper;

	std::shared_ptr<bool> cvar_enabled;

	int lastLocalScore;
	int lastEnemyScore;

	char fullDirectoryPath[MAX_PATH];

	float lastGameFrameTime = 0;
}