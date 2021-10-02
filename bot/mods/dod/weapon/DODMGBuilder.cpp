#include "DODMGBuilder.h"

#include <weapon/WeaponFunction.h>
#include <weapon/Weapon.h>

std::shared_ptr<Weapon> DODMGBuilder::build(edict_t* weap) const {
	auto weapon = DeployableWeaponBuilder::build(weap);
	weapon->getPrimary()->setFullAuto(true);
	return weapon;
}
