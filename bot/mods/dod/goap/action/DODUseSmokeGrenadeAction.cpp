#include "DODUseSmokeGrenadeAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>
#include <weapon/WeaponFunction.h>

bool DODUseSmokeGrenadeAction::precondCheck() {
	auto self = blackboard.getSelf();
	auto vision = self->getVision();
	auto target = Player::getPlayer(vision.getTargetedPlayer());
	if (target != nullptr && !target->getEdict()->IsFree() && target->isInGame()
			&& UseSpecificWeaponAction::precondCheck()
			&& arsenal.getWeapon(weapIdx) != nullptr
			&& !arsenal.getWeapon(weapIdx)->isOutOfAmmo(self->getEdict())
			&& arsenal.getWeapon(weapIdx)->isInRange(target->getCurrentPosition().DistTo(blackboard.getSelf()->getCurrentPosition()))
			&& !vision.getNearbyTeammates().empty()) {
		Vector targetLoc = target->getCurrentPosition();
		viewTarget = arsenal.getWeapon(weapIdx)->chooseWeaponFunc(self->getEdict(),
				self->getEyesPos().DistTo(targetLoc))->getAim(targetLoc, self->getEyesPos());
		return true;
	}
	return false;
}

bool DODUseSmokeGrenadeAction::execute() {
	auto self = blackboard.getSelf();
	self->setWantToListen(false);
	if (self->getAimAccuracy() < 0.9f) {
		self->setViewTarget(viewTarget);
		return false;
	}
	float dist = self->getEyesPos().DistTo(self->getViewTarget());
	Weapon *weapon = arsenal.getWeapon(weapIdx);
	if (weapon == nullptr) {
		return true;
	}
	weapon->chooseWeaponFunc(self->getEdict(),dist)->attack(blackboard.getButtons(), dist);
	return true;
}

bool DODUseSmokeGrenadeAction::canUse(const char* weaponName) const {
	return Q_strcmp(weaponName, "weapon_smoke_us") == 0
			|| Q_strcmp(weaponName, "weapon_smoke_ger") == 0;
}
