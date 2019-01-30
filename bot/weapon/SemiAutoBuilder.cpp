#include "SemiAutoBuilder.h"

#include "Weapon.h"
#include "WeaponFunction.h"

Weapon* SemiAutoBuilder::build(edict_t* weap) {
	Weapon* weapon = ReloadableWeaponBuilder::build(weap);
	weapon->setPrimary(new WeaponFunction(damage1));
	weapon->getPrimary()->getRange()[1] = 1000.0f;
	return weapon;
}
