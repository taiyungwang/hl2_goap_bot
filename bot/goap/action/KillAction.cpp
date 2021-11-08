#include "KillAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
#include <weapon/Weapon.h>

static ConVar mybotAttackDelay("mybot_attack_delay", "60");


KillAction::KillAction(Blackboard &blackboard) :
		AttackAction(blackboard) {
	effects = { WorldProp::ENEMY_SIGHTED, false };
}

bool KillAction::precondCheck() {
	framesToWait = mybotAttackDelay.GetInt();
	adjustAim = true;
	dur = 600;
	target = blackboard.getSelf()->getVision().getTargetedPlayer();
	return target > 0;
}

bool KillAction::targetDestroyed() const {
	const Player* targetedPlayer = Player::getPlayer(blackboard.getSelf()->getVision().getTargetedPlayer());
	return targetedPlayer == nullptr || !targetedPlayer->isInGame();
}

edict_t* KillAction::getTargetedEdict() const {
	if (target != blackboard.getSelf()->getVision().getTargetedPlayer()) {
		return nullptr;
	}
	const Player* player = Player::getPlayer(target);
	return player == nullptr ? nullptr : player->getEdict();
}
