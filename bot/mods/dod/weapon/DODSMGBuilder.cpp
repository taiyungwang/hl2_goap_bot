#include "DODSMGBuilder.h"

#include <weapon/WeaponFunction.h>
#include <weapon/Weapon.h>

std::shared_ptr<Weapon> DODSMGBuilder::build(edict_t* weap) const {
	auto weapon = FullAutoGunBuilder::build(weap);
	auto secondary = std::make_shared<WeaponFunction>(damage2, true);
	secondary->setMelee(true);
	secondary->setSilent(true);
	secondary->getRange()[1] = 128.0f;
	weapon->setSecondary(secondary);
	return weapon;
}
