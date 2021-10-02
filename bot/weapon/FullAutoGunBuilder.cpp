#include "FullAutoGunBuilder.h"
#include "Weapon.h"
#include "WeaponFunction.h"

std::shared_ptr<Weapon> FullAutoGunBuilder::build(edict_t* weap) const {
	auto weapon = SemiAutoBuilder::build(weap);
	weapon->getPrimary()->setFullAuto(true);
	weapon->getPrimary()->getRange()[1] = 1400.0f;
	return weapon;
}
