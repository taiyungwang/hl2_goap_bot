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
	return Player::getPlayer(blackboard.getSelf()->getVision().getTargetedPlayer()) != nullptr;
}

bool KillAction::targetDestroyed() const {
	const Player* targetedPlayer = Player::getPlayer(blackboard.getSelf()->getVision().getTargetedPlayer());
	return targetedPlayer == nullptr || !targetedPlayer->isInGame();
}

edict_t* KillAction::getTargetedEdict() const {
	auto target = Player::getPlayer(blackboard.getSelf()->getVision().getTargetedPlayer());
	return target == nullptr ? nullptr : target->getEdict();
}
