#include "CrossbowBuilder.h"

#include <weapon/Weapon.h>

CrossbowBuilder::CrossbowBuilder() :
		DeployableWeaponBuilder<Reloader>(0.8f, 100.0f, 2000.0f,
				"m_bInZoom", 1000.0f) {
}

std::shared_ptr<Weapon> CrossbowBuilder::build(edict_t* weap) const{
	auto weapon = DeployableWeaponBuilder<Reloader>::build(weap);
	weapon->getPrimary()->setSilent(true);
	weapon->setUnderWater(true);
	return weapon;
}
