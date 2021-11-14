#include "ThrowGrenadeAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
#include <weapon/Arsenal.h>
#include <weapon/WeaponFunction.h>
#include <weapon/Weapon.h>
#include <util/BaseGrenade.h>
#include <nav_mesh/nav.h>
#include <in_buttons.h>

bool ThrowGrenadeAction::precondCheck() {
	auto self = blackboard.getSelf();
	if (!UseSpecificWeaponAction::precondCheck() || arsenal.getWeapon(weapIdx) == nullptr
			|| arsenal.getWeapon(weapIdx)->isOutOfAmmo(self->getEdict())) {
		return false;
	}
	auto player = chooseTarget();
	if (player == nullptr) {
		return false;
	}
	target = arsenal.getWeapon(weapIdx)->chooseWeaponFunc(self->getEdict(),
			self->getEyesPos().DistTo(player->getCurrentPosition()))->getAim(player->getCurrentPosition(),
					self->getEyesPos());
	return true;
}

bool ThrowGrenadeAction::execute() {
	auto self = blackboard.getSelf();
	if (self->getAimAccuracy() < 0.9f) {
		self->setViewTarget(target);
		return false;
	}
	blackboard.getButtons().tap(IN_ATTACK);
	return true;
}

const Player* ThrowGrenadeAction::chooseTarget() const {
	Weapon *grenade = arsenal.getWeapon(weapIdx);
	const Player *target = nullptr;
	auto enemies = blackboard.getSelf()->getVision().getVisibleEnemies();
	float dmgRadius = BaseGrenade(grenade->getEdict()).getDmgRadius()
			+ HalfHumanWidth, minTotalDist = INFINITY;
	int maxInRange = 0;
	for (int i : enemies) {
		const Player *enemy = Player::getPlayer(i);
		if (enemy == nullptr) {
			continue;
		}
		Vector targetPos = enemy->getCurrentPosition();
		if (!grenade->isInRange(
				targetPos.DistTo(blackboard.getSelf()->getCurrentPosition()))) {
			continue;
		}
		int inRange = 0;
		for (int j : enemies) {
			const Player *enemyOther = Player::getPlayer(j);
			if (enemyOther == nullptr || enemyOther == enemy
					|| targetPos.DistTo(enemyOther->getCurrentPosition()) > dmgRadius) {
				continue;
			}
			inRange++;
		}
		if (inRange > maxInRange) {
			target = enemy;
		}
	}
	return target;
}
