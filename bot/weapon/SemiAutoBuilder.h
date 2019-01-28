#pragma once

#include "ReloadableWeaponBuilder.h"

// TODO: consider inheriting from a generic semiauto weapon.
class SemiAutoBuilder: public ReloadableWeaponBuilder<> {
public:
	SemiAutoBuilder(float damage1) {
		this->damage1 = damage1;
	}

	Weapon* build(edict_t* weap);

};
