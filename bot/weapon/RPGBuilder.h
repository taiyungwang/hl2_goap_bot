#pragma once

#include "WeaponBuilder.h"

class RPGBuilder: public WeaponBuilder {
public:
	Weapon* build(edict_t* weap);
};
