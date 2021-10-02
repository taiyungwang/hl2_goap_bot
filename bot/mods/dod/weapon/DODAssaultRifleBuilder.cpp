#include "DODAssaultRifleBuilder.h"

#include <weapon/Weapon.h>
#include <weapon/WeaponFunction.h>

std::shared_ptr<Weapon> DODAssaultRifleBuilder::build(edict_t* weap) const {
	auto weapon = DeployableWeaponBuilder<Reloader>::build(weap);
	weapon->getPrimary()->setFullAuto(true);
	return weapon;
}
