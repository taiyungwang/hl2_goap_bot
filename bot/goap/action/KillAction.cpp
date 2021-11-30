#include "KillAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>
#include <limits>

static ConVar mybotAttackDelay("mybot_attack_delay", "60");

KillAction::KillAction(Blackboard &blackboard) :
		AttackAction(blackboard) {
	effects = { WorldProp::ENEMY_SIGHTED, false };
}

bool KillAction::precondCheck() {
	framesToWait = mybotAttackDelay.GetInt();
	adjustAim = true;
	dur = UINT_MAX;
	target = blackboard.getSelf()->getVision().getTargetedPlayer();
	if (target > 0) {
		blackboard.getSelf()->setAiming(true);
		return true;
	}
	return false;
}

bool KillAction::execute()  {
	auto self = blackboard.getSelf();
	int visionTarget = self->getVision().getTargetedPlayer();
	if (target != visionTarget) {
		target = visionTarget;
		framesToWait = mybotAttackDelay.GetInt();
	}
	Weapon *weapon = self->getArsenal().getCurrWeapon();
	return ((weapon != nullptr && weapon->getDeployer() != nullptr && weapon->getMinDeployRange()
			< self->getViewTarget().DistTo(self->getCurrentPosition())
			&& !weapon->isDeployed())
			|| --framesToWait <= 0) && AttackAction::execute();
}

bool KillAction::targetDestroyed() const {
	const Player* targetedPlayer = Player::getPlayer(target);
	return targetedPlayer == nullptr || !targetedPlayer->isInGame();
}

edict_t* KillAction::getTargetedEdict() const {
	const Player* player = Player::getPlayer(target);
	return player == nullptr || !player->isInGame() ? nullptr : player->getEdict();
}
