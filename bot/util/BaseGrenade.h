#pragma once

#include "BaseCombatWeapon.h"

class BaseGrenade: public BaseCombatWeapon {
public:
	BaseGrenade(edict_t* ent): BaseCombatWeapon(ent) {
	}

	float *getDmgRadius() const {
		return getPtr<float>("m_DmgRadius");
	}

	edict_t* getThrower() const {
		return getEntity("m_hThrower");
	}

	Vector *getVelocity() const {
		return getPtr<Vector>("m_vecVelocity");
	}
};
