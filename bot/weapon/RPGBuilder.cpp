#include "RPGBuilder.h"

#include "Weapon.h"
#include "WeaponFunction.h"

std::shared_ptr<Weapon> RPGBuilder::build(edict_t* weap) const {
	auto weapon = ReloadableWeaponBuilder::build(weap);
	auto primary = std::make_shared<WeaponFunction>(0.91f);
	primary->setExplosive(true);
	primary->getRange()[0] = 400.0f;
	primary->getRange()[1] = 2000.0f;
	weapon->setPrimary(primary);
	weapon->setUnderWater(true);
	return weapon;
}
