#include "MeleeWeaponBuilder.h"

#include "Weapon.h"
#include "WeaponFunction.h"

Weapon* MeleeWeaponBuilder::build(edict_t* weap) {
	Weapon* weapon = new Weapon(weap);
	WeaponFunction *primary = new WeaponFunction(0.6f);
	weapon->setUnderWater(true);
	primary->setMelee(true);
	primary->setFullAuto(true);
	primary->getRange()[1] = 50.0f;
	weapon->setPrimary(primary);
	return weapon;
}
