#include "GrenadeBuilder.h"

#include "Weapon.h"
#include "WeaponFunction.h"

std::shared_ptr<Weapon> GrenadeBuilder::build(edict_t* weap) const {
	auto weapon = std::make_shared<Weapon>(weap);
	auto primary = std::make_shared<WeaponFunction>(0.9f);
	weapon->setGrenade(true);
	primary->setExplosive(true);
	primary->setSilent(true);
	primary->getRange()[0] = 250.0f;
	primary->getRange()[1] = 330.0f;
	weapon->setPrimary(primary);
	return weapon;
}
