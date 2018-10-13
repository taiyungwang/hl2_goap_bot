#pragma once

#include <weapon/WeaponBuilder.h>

class GravityGunBuilder: public WeaponBuilder {
public:
	Weapon* build(edict_t* weap);

};
