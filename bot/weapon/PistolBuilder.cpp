#include "PistolBuilder.h"

#include "Weapon.h"
#include "WeaponFunction.h"

std::shared_ptr<Weapon> PistolBuilder::build(edict_t* weap) const {
	auto weapon = SemiAutoBuilder::build(weap);
	weapon->setUnderWater(true);
	return weapon;
}
