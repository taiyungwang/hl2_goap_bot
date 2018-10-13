#include "MagnumBuilder.h"

#include <weapon/Weapon.h>
#include <weapon/WeaponFunction.h>

Weapon* MagnumBuilder::build(edict_t* weap) {
	Weapon* weapon = SemiAutoBuilder::build(weap);
	weapon->getPrimary()->getRange()[0] = 100.0f;
	return weapon;
}
