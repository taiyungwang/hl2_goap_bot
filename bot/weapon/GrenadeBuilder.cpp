#include "GrenadeBuilder.h"

#include "Weapon.h"
#include "WeaponFunction.h"

Weapon* GrenadeBuilder::build(edict_t* weap) {
	Weapon* weapon = new Weapon(weap);
	WeaponFunction *primary = new WeaponFunction(0.9f);
	weapon->setGrenade(true);
	primary->setExplosive(true);
	primary->getRange()[0] = 250.0f;
	primary->getRange()[1] = 330.0f;
	weapon->setPrimary(primary);
	return weapon;
}
