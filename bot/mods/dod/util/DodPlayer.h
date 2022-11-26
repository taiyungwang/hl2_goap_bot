#pragma once

#include <util/BasePlayer.h>

class DodPlayer: public BasePlayer {
public:
	DodPlayer(edict_t *ent): BasePlayer(ent) {
	}

	int getPlayerClass() const {
		return get<int>("m_iPlayerClass", -1);
	}

	bool isProne() const {
		return *getPtr<bool>("m_bProne");
	}

	bool isDefusing() const {
		return *getPtr<bool>("m_bDefusing");
	}

	bool isPlanting() const {
		return *getPtr<bool>("m_bPlanting");
	}
};
