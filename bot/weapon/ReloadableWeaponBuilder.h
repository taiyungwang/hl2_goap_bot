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
	virtual std::shared_ptr<Weapon> build(edict_t* weap) const {
		auto weapon = std::make_shared<Weapon>(weap);
		weapon->setReloader<T>();
		return weapon;
	}
};
