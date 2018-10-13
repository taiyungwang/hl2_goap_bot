#pragma once

#include "EntityInstance.h"

class BaseCombatWeapon: public EntityInstance {
public:
	BaseCombatWeapon(edict_t *ent) :
			EntityInstance(ent, "CBaseCombatWeapon") {
	}

	int getWeaponState() {
		return get<int>("m_iState");
	}

	int getAmmoType(int i) {
		return *(getPtr<int>(AMMO_TYPES[i]));
	}

	int *getClipIndex(int i) {
		return getPtr<int>(CLIP_NAMES[i]);
	}

private:
	static const char* CLIP_NAMES[2];
	static const char* AMMO_TYPES[2];
};
