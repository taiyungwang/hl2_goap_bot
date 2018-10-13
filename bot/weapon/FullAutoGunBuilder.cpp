#include "FullAutoGunBuilder.h"
#include "Weapon.h"
#include "WeaponFunction.h"

Weapon* FullAutoGunBuilder::build(edict_t* weap) {
	Weapon* weapon = SemiAutoBuilder::build(weap);
	weapon->getPrimary()->setFullAuto(true);
	weapon->getPrimary()->getRange()[1] = 1400.0f;
	return weapon;
}
