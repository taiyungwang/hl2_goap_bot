#include "GrenadeBuilder.h"

#include "Weapon.h"
#include "GrenadeLauncherFunction.h"
#include <util/BaseGrenade.h>

std::shared_ptr<Weapon> GrenadeBuilder::build(edict_t* weap) const {
	auto weapon = std::make_shared<Weapon>(weap);
	auto primary = std::make_shared<GrenadeLauncherFunction>();
	weapon->setGrenade(true);
	primary->setExplosive(true);
	primary->setSilent(true);
	primary->getRange()[0] = BaseGrenade(weap).getDmgRadius();
	primary->getRange()[1] = 800.0f;
	primary->setZMultiplier(zMultiplier);
	weapon->setPrimary(primary);
	return weapon;
}
