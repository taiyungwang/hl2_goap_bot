#include "DODUseSmokeGrenadeAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>
#include <weapon/WeaponFunction.h>

bool DODUseSmokeGrenadeAction::precondCheck() {
	auto self = blackboard.getSelf();
	auto vision = self->getVision();
	if (UseSpecificWeaponAction::precondCheck()
			&& arsenal.getWeapon(weapIdx) != nullptr
			&& !arsenal.getWeapon(weapIdx)->isOutOfAmmo(self->getEdict())
			&& !vision.getNearbyTeammates().empty()
			&& vision.getVisibleEnemies().size() > 1) {
		chooseTarget();
		if (target == nullptr) {
			return false;
		}
		viewTarget = target->getCurrentPosition();
		if (!arsenal.getWeapon(weapIdx)->isInRange(viewTarget.DistTo(blackboard.getSelf()->getCurrentPosition()))) {
			return false;
		}
		viewTarget = arsenal.getWeapon(weapIdx)->chooseWeaponFunc(self->getEdict(),
				self->getEyesPos().DistTo(viewTarget))->getAim(viewTarget, self->getEyesPos());
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

void DODUseSmokeGrenadeAction::chooseTarget() {
	float closest = INFINITY;
	auto self = blackboard.getSelf();
	auto vision = self->getVision();
	for (auto i: vision.getVisibleEnemies()) {
		auto enemy = Player::getPlayer(i);
		float dist = self->getCurrentPosition().DistTo(enemy->getCurrentPosition());
		if (dist < closest) {
			target = enemy;
			closest = dist;
		}
	}
}
