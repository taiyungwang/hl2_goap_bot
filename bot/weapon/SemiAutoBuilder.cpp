#include "SemiAutoBuilder.h"

#include "Weapon.h"
#include "WeaponFunction.h"

std::shared_ptr<Weapon> SemiAutoBuilder::build(edict_t* weap) const {
	auto weapon = ReloadableWeaponBuilder::build(weap);
	weapon->setPrimary(std::make_shared<WeaponFunction>(damage1));
	weapon->getPrimary()->getRange()[1] = 1000.0f;
	return weapon;
}
