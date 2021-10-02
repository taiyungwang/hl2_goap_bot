#include "MeleeWeaponBuilder.h"

#include "Weapon.h"
#include "WeaponFunction.h"

std::shared_ptr<Weapon> MeleeWeaponBuilder::build(edict_t* weap) const {
	auto weapon = std::make_shared<Weapon>(weap);
	auto primary = std::make_shared<WeaponFunction>(0.1f);
	weapon->setUnderWater(true);
	primary->setMelee(true);
	primary->setFullAuto(true);
	primary->setSilent(true);
	primary->getRange()[1] = 300.0f;
	weapon->setPrimary(primary);
	return weapon;
}
