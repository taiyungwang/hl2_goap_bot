#include "KillAction.h"

#include <player/Blackboard.h>
#include <player/Player.h>
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
	return blackboard.getTargetedPlayer() != nullptr;
}

bool KillAction::targetDestroyed() const {
	return blackboard.getTargetedPlayer() == nullptr
			|| !blackboard.getTargetedPlayer()->isInGame();
}

edict_t* KillAction::getTargetedEdict() const {
	return blackboard.getTargetedPlayer()->getEdict();
}
