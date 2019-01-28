#pragma once

#include "ReloadableWeaponBuilder.h"

class RPGBuilder: public ReloadableWeaponBuilder<> {
public:
	Weapon* build(edict_t* weap);
};
