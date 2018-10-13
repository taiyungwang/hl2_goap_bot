#include "DODUseSmokeGrenadeAction.h"

#include <player/Blackboard.h>
#include <player/Player.h>
#include <weapon/Weapon.h>
#include <weapon/WeaponFunction.h>

DODUseSmokeGrenadeAction::DODUseSmokeGrenadeAction(Blackboard& blackboard) :
		UseSpecificWeaponAction(blackboard) {
	effects = {WorldProp::ENEMY_SIGHTED, false};
}

bool DODUseSmokeGrenadeAction::precondCheck() {
	return UseSpecificWeaponAction::precondCheck()
			&& blackboard.getTargetedPlayer() != nullptr
			&& !blackboard.getTargetedPlayer()->isDead();
}

bool DODUseSmokeGrenadeAction::execute() {
	auto target = blackboard.getTargetedPlayer();
	if (target == nullptr || target->isDead()) {
		return true;
	}
	auto self = blackboard.getSelf();
	Vector targetLoc = target->getCurrentPosition();
	float dist = self->getEyesPos().DistTo(targetLoc);
	WeaponFunction* grenade = armory.getWeapon(weapIdx)->chooseWeaponFunc(
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
