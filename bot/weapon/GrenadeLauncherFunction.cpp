#include "GrenadeLauncherFunction.h"

#include <player/Buttons.h>
#include <nav_mesh/nav.h>
#include <vector.h>
#include <convar.h>
#include <in_buttons.h>

GrenadeLauncherFunction::GrenadeLauncherFunction() :
		WeaponFunction(0.9, true) {
	range[0] = 250.0f;
	range[1] = 700.0f;
	explosive = true;
}

Vector GrenadeLauncherFunction::getAim(const Vector& target,
		const Vector& eye) const {
	extern ICvar* g_pCVar;
	float dist = target.AsVector2D().DistTo(eye.AsVector2D());
	return Vector(target.x, target.y,
			dist > range[1] ? dist
					: (target.z + dist / range[1] * g_pCVar->FindVar("sv_gravity")->GetFloat())
					  - eye.z + target.z + HumanEyeHeight);
}

void GrenadeLauncherFunction::attack(Buttons& buttons, float distance) const {
	buttons.hold(IN_ATTACK);
}
