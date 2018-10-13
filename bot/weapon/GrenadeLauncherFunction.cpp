#include "GrenadeLauncherFunction.h"

#include <player/Buttons.h>
#include <vector.h>
#include <in_buttons.h>

GrenadeLauncherFunction::GrenadeLauncherFunction() :
		WeaponFunction(0.9, true) {
	range[0] = 250.0f;
	range[1] = 700.0f;
	explosive = true;
}

Vector GrenadeLauncherFunction::getAim(const Vector& target,
		const Vector& eye) const {
	Vector aim = target;
	aim.z += zMultiplier * (target.DistTo(eye) - range[0])
			/ (range[1] - range[0]);
	return aim;
}

void GrenadeLauncherFunction::attack(Buttons& buttons, float distance) const {
	buttons.hold(IN_ATTACK);
}
