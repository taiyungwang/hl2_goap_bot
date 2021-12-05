#include "GrenadeBuilder.h"

#include "Weapon.h"
#include "GrenadeLauncherFunction.h"
#include <nav_mesh/nav.h>
#include <convar.h>

std::shared_ptr<Weapon> GrenadeBuilder::build(edict_t* weap) const {
	auto weapon = std::make_shared<Weapon>(weap);
	auto primary = std::make_shared<GrenadeLauncherFunction>();
	weapon->setGrenade(true);
	primary->setExplosive(true);
	primary->setSilent(true);
	extern ICvar* cVars;
	float g = cVars->FindVar("sv_gravity")->GetFloat();
	primary->getRange()[0] = 400.0f; // range from DOD, maybe not appropriate for all MODS
	primary->getRange()[1] = initialSpeed * (initialSpeed
			+ std::sqrt(initialSpeed * initialSpeed + 2.0f * g * HumanEyeHeight)) / g;
	primary->setZMultiplier(zMultiplier);
	weapon->setPrimary(primary);
	return weapon;
}
