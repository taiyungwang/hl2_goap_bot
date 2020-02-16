#pragma once

#include <util/BaseEntity.h>

class DODBombTarget: public BaseEntity {
public:
	DODBombTarget(edict_t *ent): BaseEntity("CDODBombTarget", ent) {
	}

	int getState() {
		return get<int>("m_iState");
	}

	int getBombingTeam() {
		return get<int>("m_iBombingTeam");

	}
};
