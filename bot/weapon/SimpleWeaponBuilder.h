#pragma once

#include "WeaponBuilder.h"
#include "Weapon.h"

/**
 * A weapon builder.
 */
template<typename T>
class SimpleWeaponBuilder: public WeaponBuilder {
public:
	Weapon* build(edict_t* weap) {
		Weapon* weapon = new Weapon(weap);
		weapon->setPrimary(new T());
		return weapon;
	}
};
