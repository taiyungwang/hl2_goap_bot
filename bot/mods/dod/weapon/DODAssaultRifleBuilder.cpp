#include "DODAssaultRifleBuilder.h"

#include <weapon/Weapon.h>
#include <weapon/WeaponFunction.h>

Weapon* DODAssaultRifleBuilder::build(edict_t* weap) {
	Weapon* weapon = DeployableWeaponBuilder::build(weap);
	weapon->getPrimary()->setFullAuto(true);
	return weapon;
}
