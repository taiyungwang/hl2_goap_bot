#include "PistolBuilder.h"

#include "Weapon.h"
#include "WeaponFunction.h"

Weapon* PistolBuilder::build(edict_t* weap) {
	Weapon* weapon = SemiAutoBuilder::build(weap);
	weapon->setUnderWater(true);
	return weapon;
}
