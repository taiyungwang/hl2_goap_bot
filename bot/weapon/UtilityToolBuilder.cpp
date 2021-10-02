#include "UtilityToolBuilder.h"

#include "Weapon.h"
#include "WeaponFunction.h"

std::shared_ptr<Weapon> UtilityToolBuilder::build(edict_t* weap) const {
	auto weapon = std::make_shared<Weapon>(weap);
	weapon->setPrimary(std::make_shared<WeaponFunction>(0.0f));
	weapon->getPrimary()->getRange()[1] = maxRange;
	return weapon;
}
