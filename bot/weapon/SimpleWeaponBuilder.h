#pragma once

#include "ReloadableWeaponBuilder.h"
#include "Weapon.h"

/**
 * A weapon builder.
 */
template<typename T>
class SimpleWeaponBuilder: public ReloadableWeaponBuilder<> {
public:
	Weapon* build(edict_t* weap) {
		Weapon* weapon = new Weapon(weap);
		weapon->setPrimary(new T());
		return weapon;
	}
};
