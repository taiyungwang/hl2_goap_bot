#pragma once

#include "WeaponBuilder.h"

class GrenadeBuilder: public WeaponBuilder {
public:
	Weapon* build(edict_t* weap);

};
