#pragma once

#include <util/EntityInstance.h>

class DODBombTarget: public EntityInstance {
public:
	DODBombTarget(edict_t *ent) :
			EntityInstance(ent, "CDODBombTarget") {
	}

	int getState() {
		return get<int>("m_iState");
	}

	int getBombingTeam() {
		return get<int>("m_iBombingTeam");

	}
};
