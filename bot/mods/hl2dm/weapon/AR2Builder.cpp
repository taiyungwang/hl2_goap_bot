#include "AR2Builder.h"

#include <weapon/WeaponFunction.h>
#include <weapon/Weapon.h>

std::shared_ptr<Weapon> AR2Builder::build(edict_t* weap) const {
	auto weapon = FullAutoGunBuilder::build(weap);
	auto secondary = std::make_shared<WeaponFunction>(damage2, true);
	secondary->getRange()[1] = 2000.0f;
	weapon->setSecondary(secondary);
	return weapon;
}
