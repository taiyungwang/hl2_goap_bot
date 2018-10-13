#include "RPGBuilder.h"

#include "Weapon.h"
#include "WeaponFunction.h"

Weapon* RPGBuilder::build(edict_t* weap) {
	Weapon* weapon = new Weapon(weap);
	WeaponFunction *primary = new WeaponFunction(0.91f);
	primary->setExplosive(true);
	primary->getRange()[0] = 400.0f;
	primary->getRange()[1] = 2000.0f;
	weapon->setPrimary(primary);
	weapon->setUnderWater(true);
	return weapon;
}
