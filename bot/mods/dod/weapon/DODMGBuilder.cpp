#include "DODMGBuilder.h"

#include <weapon/WeaponFunction.h>
#include <weapon/Weapon.h>

Weapon* DODMGBuilder::build(edict_t* weap) {
	Weapon* weapon = DeployableWeaponBuilder::build(weap);
	weapon->getPrimary()->setFullAuto(true);
	return weapon;
}
