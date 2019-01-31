#pragma once

#include <util/EntityInstance.h>

class DodPlayer: public EntityInstance {
public:
	DodPlayer(edict_t *ent) :
			EntityInstance(ent, "CDODPlayer") {
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
