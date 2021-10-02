#pragma once

#include "ReloadableWeaponBuilder.h"
#include "Weapon.h"

/**
 * A weapon builder.
 */
template<typename T>
class SimpleWeaponBuilder: public ReloadableWeaponBuilder<> {
public:
	virtual std::shared_ptr<Weapon> build(edict_t* weap) const {
		auto weapon = ReloadableWeaponBuilder::build(weap);
		weapon->setPrimary(std::make_shared<T>());
		return weapon;
	}
};
