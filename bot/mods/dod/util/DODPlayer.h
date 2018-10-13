#pragma once

#include <util/EntityInstance.h>

class DODPlayer: public EntityInstance {
public:
	DODPlayer(edict_t *ent) :
			EntityInstance(ent, "CDODPlayer") {
	}

	int getPlayerClass() {
		return get<int>("m_iPlayerClass");
	}

	bool isProne() {
		return get<bool>("m_bProne");
	}
};
