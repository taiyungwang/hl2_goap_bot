#include "MeleeWeaponBuilder.h"

#include "Weapon.h"
#include "WeaponFunction.h"

Weapon* MeleeWeaponBuilder::build(edict_t* weap) {
	Weapon* weapon = new Weapon(weap);
	WeaponFunction *primary = new WeaponFunction(0.1f);
	weapon->setUnderWater(true);
	primary->setMelee(true);
	primary->setFullAuto(true);
	primary->setSilent(true);
	primary->getRange()[1] = 300.0f;
	weapon->setPrimary(primary);
	return weapon;
}
