#include "ThrowGrenadeAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
#include <weapon/WeaponFunction.h>
#include <weapon/Weapon.h>
#include <nav_mesh/nav.h>
#include <in_buttons.h>

bool ThrowGrenadeAction::precondCheck() {
	auto self = blackboard.getSelf();
	auto weapon = self->getWeapon(weapIdx);
	if (!weapon || !UseSpecificWeaponAction::precondCheck() || self->getWeapon(weapIdx) == nullptr
			|| weapon->isOutOfAmmo(self->getEdict())) {
		return false;
	}
	auto player = chooseTarget();
	if (player == nullptr) {
		return false;
	}
	target = weapon->chooseWeaponFunc(self->getEdict(),
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
	auto grenade = blackboard.getSelf()->getWeapon(weapIdx);
	const Player *target = nullptr;
	auto &vision = blackboard.getSelf()->getVision();
	auto enemies = vision.getVisibleEnemies();
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
					|| targetPos.DistTo(enemyOther->getCurrentPosition())
					// TODO: dod range, maybe not appropriate for all mods
					> 400.0f) {
				continue;
			}
			inRange++;
		}
		if (inRange > maxInRange
				// use grenade if no LOS to any enemies.
				|| (target == nullptr && vision.getTargetedPlayer() == 0)) {
			target = enemy;
		}
	}
	return target;
}

bool ThrowGrenadeAction::canUse(int weapIndx) const {
	extern IVEngineServer* engine;
	return UseSpecificWeaponAction::canUse(weapIndx)
			&& canUse(engine->PEntityOfEntIndex(weapIdx)->GetClassName());
}
