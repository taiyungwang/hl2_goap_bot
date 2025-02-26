#include "GrenadeBuilder.h"

#include "Weapon.h"
#include "GrenadeLauncherFunction.h"
#include <nav_mesh/nav.h>
#include <convar.h>
#include <cmath>

std::shared_ptr<Weapon> GrenadeBuilder::build(edict_t* weap) const {
	auto weapon = std::make_shared<Weapon>(weap);
	auto primary = std::make_shared<GrenadeLauncherFunction>();
	weapon->setGrenade(true);
	primary->setExplosive(true);
	primary->setSilent(true);
	extern ICvar* g_pCVar;
	// V² = Vx² + Vy²
	// Vx = Vy because 45 degree angle for max range.
	// V² = 2 * Vx²
	// Vx = √(V² / 2)
	// R = Vx * [Vy + √(Vy² + 2 * g * h)] / g
	float g = g_pCVar->FindVar("sv_gravity")->GetFloat(),
			zHoriz = std::sqrt(initialSpeed * initialSpeed / 2.0f);
	primary->getRange()[0] = 400.0f; // range from DOD, maybe not appropriate for all MODS
	primary->getRange()[1] = zHoriz * (zHoriz
			+ std::sqrt(zHoriz * zHoriz + 2.0f * g * HumanEyeHeight)) / g;
	primary->setInitialVelocity(initialSpeed);
	weapon->setPrimary(primary);
	return weapon;
}
