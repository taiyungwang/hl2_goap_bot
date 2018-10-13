#pragma once

#include "WeaponBuilder.h"

class MeleeWeaponBuilder: public WeaponBuilder {
public:
	Weapon* build(edict_t* weap);

};
