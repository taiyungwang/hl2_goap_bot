#include "AR2Builder.h"

#include <weapon/WeaponFunction.h>
#include <weapon/Weapon.h>

Weapon* AR2Builder::build(edict_t* weap) {
	Weapon* weapon = FullAutoGunBuilder::build(weap);
	WeaponFunction* secondary = new WeaponFunction(damage2, true);
	secondary->getRange()[1] = 2000.0f;
	weapon->setSecondary(secondary);
	return weapon;
}
