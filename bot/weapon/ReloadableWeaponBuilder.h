#pragma once

#include "WeaponBuilder.h"
#include "Reloader.h"
#include "Weapon.h"

/**
 * Builds a weapon that requires reloading.
 */
template<typename T= Reloader>
class ReloadableWeaponBuilder: public WeaponBuilder {
public:
	virtual Weapon* build(edict_t* weap) {
		Weapon* weapon = new Weapon(weap);
		weapon->setReloader(new T(*weapon));
		return weapon;
	}
};
