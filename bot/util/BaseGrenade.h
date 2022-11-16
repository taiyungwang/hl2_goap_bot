#pragma once

#include "BaseCombatWeapon.h"

class BaseGrenade: public BaseCombatWeapon {
public:
	BaseGrenade(edict_t* ent): BaseCombatWeapon(ent) {
	}

	float getDmgRadius() {
		return get<float>("m_DmgRadius");
	}

	edict_t* getThrower() {
		return getEntity("m_hThrower");
	}

	Vector getVelocity() {
		return get<Vector>("m_vecVelocity");
	}
};
