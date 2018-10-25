#include "UtilityToolBuilder.h"

#include "Weapon.h"
#include "WeaponFunction.h"

Weapon* UtilityToolBuilder::build(edict_t* weap) {
	Weapon* weapon = new Weapon(weap);
	weapon->setPrimary(new WeaponFunction(0.0f));
	weapon->getPrimary()->getRange()[1] = maxRange;
	return weapon;
}
