#include "DODUseSmokeGrenadeAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
#include <weapon/Weapon.h>
#include <weapon/WeaponFunction.h>

bool DODUseSmokeGrenadeAction::precondCheck() {
	auto target = blackboard.getTargetedPlayer();
	return target != nullptr && !target->getEdict()->IsFree() && !target->isDead()
			&& UseSpecificWeaponAction::precondCheck()
			&& arsenal.getWeapon(weapIdx) != nullptr
			&& arsenal.getWeapon(weapIdx)->isInRange(target->getCurrentPosition().DistTo(blackboard.getSelf()->getCurrentPosition()));
}

bool DODUseSmokeGrenadeAction::execute() {
	if (!precondCheck()) {
		return true;
	}
	auto self = blackboard.getSelf();
	Vector targetLoc = blackboard.getTargetedPlayer()->getCurrentPosition();
	float dist = self->getEyesPos().DistTo(targetLoc);
	WeaponFunction* grenade = arsenal.getWeapon(weapIdx)->chooseWeaponFunc(
			self->getEdict(), dist);
	blackboard.setViewTarget(
			grenade->getAim(targetLoc, self->getEyesPos()));
	if (blackboard.getAimAccuracy(targetLoc) < 0.9f) {
		return false;
	}
	grenade->attack(blackboard.getButtons(), dist);
	return true;
}

bool DODUseSmokeGrenadeAction::canUse(const char* weaponName) const {
	return Q_strcmp(weaponName, "weapon_smoke_us") == 0
			|| Q_strcmp(weaponName, "weapon_smoke_ger") == 0;
}
