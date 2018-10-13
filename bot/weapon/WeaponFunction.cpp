#include "WeaponFunction.h"

#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <util/BasePlayer.h>
#include <in_buttons.h>

WeaponFunction::WeaponFunction(float damage, bool isSecondary) :
		button(isSecondary ? IN_ATTACK2 : IN_ATTACK), damageRating(damage), clipId(
				-1), range { 0.f, 0.f } {
	explosive = melee = fullAuto = false;
}

void WeaponFunction::attack(Buttons& buttons, float distance) const {
	if (fullAuto && distance < 500.0f) {
		buttons.hold(button);
	} else {
		buttons.tap(button);
	}
}

int WeaponFunction::getAmmo(edict_t* player) const {
	return clipId == -1 ? 0 : BasePlayer(player).getAmmo()[clipId];
}

Vector WeaponFunction::getAim(const Vector& target,
		const Vector& eyes) const {
	return target;
}
