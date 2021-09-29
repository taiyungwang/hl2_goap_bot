#include "DODSMGBuilder.h"

#include <weapon/WeaponFunction.h>
#include <weapon/Weapon.h>

Weapon* DODSMGBuilder::build(edict_t* weap) {
	Weapon* weapon = FullAutoGunBuilder::build(weap);
	WeaponFunction* secondary = new WeaponFunction(damage2, true);
	secondary->setMelee(true);
	secondary->setSilent(true);
	secondary->getRange()[1] = 128.0f;
	weapon->setSecondary(secondary);
	return weapon;
}
