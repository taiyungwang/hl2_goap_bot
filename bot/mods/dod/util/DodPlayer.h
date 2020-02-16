#pragma once

#include <util/BasePlayer.h>

class DodPlayer: public BasePlayer {
public:
	DodPlayer(edict_t *ent): BasePlayer("CDODPlayer", ent) {
	}

	int getPlayerClass() {
		return get<int>("m_iPlayerClass");
	}

	bool isProne() {
		return get<bool>("m_bProne");
	}

	bool isDefusing() {
		return get<bool>("m_bDefusing");
	}

	bool isPlanting() {
		return get<bool>("m_bPlanting");
	}
};
