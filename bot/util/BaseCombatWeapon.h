#pragma once

#include <util/BaseEntity.h>

class BaseCombatWeapon: public BaseEntity {
public:
	BaseCombatWeapon(edict_t *ent): BaseEntity(ent) {
	}

	int getWeaponState() const {
		return get<int>("m_iState", 0);
	}

	int getAmmoType(int i) const {
		return *(getPtr<int>(AMMO_TYPES[i]));
	}

	int *getClipIndex(int i) const {
		return getPtr<int>(CLIP_TYPES[i]);
	}

	edict_t *getOwner() const {
		return getEntity("m_hOwner");
	}

private:
	static const char* CLIP_TYPES[2];
	static const char* AMMO_TYPES[2];
};
