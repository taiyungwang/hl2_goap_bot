#include "AttackAction.h"

#include <player/Blackboard.h>
#include <player/Player.h>

AttackAction::AttackAction(Blackboard& blackboard) :
		DestroyObjectAction(blackboard) {
	effects = {WorldProp::ENEMY_SIGHTED, false};
}

bool AttackAction::precondCheck() {
	adjustAim = true;
	dur = 600;
	return blackboard.getTargetedPlayer() != nullptr;
}

bool AttackAction::postCondCheck() {
	return blackboard.getTargetedPlayer() == nullptr
			|| blackboard.getTargetedPlayer()->isDead();
}

edict_t* AttackAction::getTargetedEdict() const {
	return blackboard.getTargetedPlayer()->getEdict();
}
