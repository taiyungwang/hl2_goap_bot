#include "KillAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
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
	dur = std::numeric_limits<unsigned int>::max();
	target = blackboard.getSelf()->getVision().getTargetedPlayer();
	return target > 0;
}

bool KillAction::execute()  {
	int visionTarget = blackboard.getSelf()->getVision().getTargetedPlayer();
	if (target != visionTarget) {
		target = visionTarget;
		framesToWait = mybotAttackDelay.GetInt();
	}
	return --framesToWait <= 0 && AttackAction::execute();
}

bool KillAction::targetDestroyed() const {
	const Player* targetedPlayer = Player::getPlayer(target);
	return targetedPlayer == nullptr || !targetedPlayer->isInGame();
}

edict_t* KillAction::getTargetedEdict() const {
	const Player* player = Player::getPlayer(target);
	return player == nullptr || !player->isInGame() ? nullptr : player->getEdict();
}
