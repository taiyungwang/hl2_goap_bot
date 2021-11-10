#pragma once

#include <util/BaseEntity.h>

class BaseGrenade: public BaseEntity {
public:
	BaseGrenade(edict_t* ent): BaseEntity("CBaseGrenade", ent) {
	}

	float getDmgRadius() {
		return get<float>("m_DmgRadius");
	}

	edict_t* getThrower() {
		return getEntity("m_hThrower");
	}
};
