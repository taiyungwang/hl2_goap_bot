#include "GravityGunBuilder.h"

#include <weapon/Weapon.h>
#include <weapon/WeaponFunction.h>

Weapon* GravityGunBuilder::build(edict_t* weap) {
	Weapon* weapon = new Weapon(weap);
	weapon->setPrimary(new WeaponFunction(0.0f));
	weapon->getPrimary()->getRange()[1] = 768.0f;
	return weapon;
}
