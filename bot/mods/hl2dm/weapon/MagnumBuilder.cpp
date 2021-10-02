#include "MagnumBuilder.h"

#include <weapon/Weapon.h>
#include <weapon/WeaponFunction.h>

std::shared_ptr<Weapon> MagnumBuilder::build(edict_t* weap) const {
	auto weapon = SemiAutoBuilder::build(weap);
	weapon->getPrimary()->getRange()[0] = 100.0f;
	return weapon;
}
