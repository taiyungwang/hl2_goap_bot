#pragma once

#include <weapon/DeployableWeaponBuilder.h>

class CrossbowBuilder: public DeployableWeaponBuilder {
public:
	CrossbowBuilder();

	Weapon* build(edict_t* weap);
};
