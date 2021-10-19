#include "KillAction.h"

#include <player/Blackboard.h>
#include <player/Player.h>
#include <weapon/Weapon.h>

KillAction::KillAction(Blackboard &blackboard) :
		AttackAction(blackboard) {
	effects = { WorldProp::ENEMY_SIGHTED, false };
}

bool KillAction::precondCheck() {
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
