#pragma once

#include <weapon/Reloader.h>
#include <weapon/DeployableWeaponBuilder.h>

class CrossbowBuilder: public DeployableWeaponBuilder<Reloader> {
public:
	CrossbowBuilder();

	Weapon* build(edict_t* weap);
};
