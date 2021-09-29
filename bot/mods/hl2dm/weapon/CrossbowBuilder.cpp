#include "CrossbowBuilder.h"

#include <weapon/Weapon.h>

CrossbowBuilder::CrossbowBuilder() :
		DeployableWeaponBuilder<Reloader>(0.8f, 100.0f, 2000.0f, "CWeaponCrossbow",
				"m_bInZoom", 1000.0f) {
}

Weapon* CrossbowBuilder::build(edict_t* weap) {
	Weapon* weapon = DeployableWeaponBuilder<Reloader>::build(weap);
	weapon->getPrimary()->setSilent(true);
	weapon->setUnderWater(true);
	return weapon;
}
