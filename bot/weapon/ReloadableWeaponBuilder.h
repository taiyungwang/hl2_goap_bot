#pragma once

#include "WeaponBuilder.h"
#include "Reloader.h"
#include "Weapon.h"

template<typename T= Reloader>
class ReloadableWeaponBuilder: public WeaponBuilder {
public:
	virtual Weapon* build(edict_t* weap) {
		Weapon* weapon = new Weapon(weap);
		weapon->setReloader(new T(*weapon));
		return weapon;
	}
};
