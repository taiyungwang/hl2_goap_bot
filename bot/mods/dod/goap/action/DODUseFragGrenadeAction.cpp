#include "DODUseFragGrenadeAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
#include <player/Vision.h>
#include <weapon/Weapon.h>
#include <weapon/WeaponFunction.h>

bool DODUseFragGrenadeAction::precondCheck() {
	bool success = DODUseRifleGrenadeAction::precondCheck();
	if (success) {
		primeDuration = 0;
	}
	return success;
}

// TODO: change to using player index in case of player logging off.
bool DODUseFragGrenadeAction::execute() {
	if (target->getEdict()->IsFree() || target->isDead()) {
		return true;
	}
	float dist = blackboard.getTargetEntDistance();
	const Player* self = blackboard.getSelf();
	WeaponFunction* grenade = armory.getWeapon(weapIdx)->chooseWeaponFunc(
			self->getEdict(), dist);
	blackboard.setViewTarget(
			grenade->getAim(target->getCurrentPosition(), self->getEyesPos()));
	if (primeDuration++ >= 300
			|| !grenade->isInRange(
					target->getCurrentPosition().DistTo(
							self->getCurrentPosition()))
			|| !UTIL_IsVisible(blackboard.getViewTarget(), blackboard,
					target->getEdict())) {
		return true;
	}
	grenade->attack(blackboard.getButtons(), dist);
	return false;
}

bool DODUseFragGrenadeAction::canUse(const char* weaponName) const {
	return Q_strcmp(weaponName, "weapon_frag_us") == 0
			|| Q_strcmp(weaponName, "weapon_frag_ger") == 0;
}
