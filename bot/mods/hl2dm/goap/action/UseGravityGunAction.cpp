#include "UseGravityGunAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
#include <player/Vision.h>
#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>
#include <weapon/WeaponFunction.h>

bool UseGravityGunAction::precondCheck() {
	return UseSpecificWeaponAction::precondCheck()
			&& Q_stristr(blackboard.getBlocker()->GetClassName(), "physics")
					!= nullptr;
}

bool UseGravityGunAction::execute() {
	edict_t* blocker = blackboard.getBlocker();
	if (blocker == nullptr) {
		return true;
	}
	Bot* self = blackboard.getSelf();
	float dist = self->getViewTarget().DistTo(self->getCurrentPosition());
	if (dist > 130.0f) {
		blackboard.setBlocker(nullptr);
		return true;
	}
	if (self->getAimAccuracy()
			> 1.0f - 30.0f / (dist == 0.0f ? 0.0001f : dist)) {
		arsenal.getCurrWeapon()->getPrimary()->attack(blackboard.getButtons(),
				dist);
	}
	return false;
}

bool UseGravityGunAction::canUse(const char* weaponName) const {
	return Q_strcmp(weaponName, "weapon_physcannon") == 0;
}
