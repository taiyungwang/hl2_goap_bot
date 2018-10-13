#include "UseGravityGunAction.h"

#include <player/Blackboard.h>
#include <player/Player.h>
#include <player/Vision.h>
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
	Vector targetLoc =
			blocker->GetCollideable()->GetCollisionOrigin();
	float dist = targetLoc.DistTo(blackboard.getSelf()->getCurrentPosition());
	blackboard.setViewTarget(targetLoc);
	if (dist > 130.0f || !UTIL_IsVisible(targetLoc, blackboard, blocker)) {
		blackboard.setBlocker(nullptr);
		return true;
	}
	if (fabs(blackboard.getAimAccuracy(targetLoc))
			> 1.0f - 30.0f / (dist == 0.0f ? 0.0001f : dist)) {
		armory.getCurrWeapon()->getPrimary()->attack(blackboard.getButtons(),
				dist);
	}
	return false;
}

bool UseGravityGunAction::canUse(const char* weaponName) const {
	return Q_strcmp(weaponName, "weapon_physcannon") == 0;
}
