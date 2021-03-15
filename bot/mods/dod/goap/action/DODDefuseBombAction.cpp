#include "DODDefuseBombAction.h"

#include <mods/dod/player/DODObjectives.h>
#include <mods/dod/player/DODObjective.h>
#include <mods/dod/util/DodPlayer.h>
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <nav_mesh/nav.h>
#include <util/UtilTrace.h>
#include <in_buttons.h>

DODDefuseBombAction::DODDefuseBombAction(Blackboard& blackboard) :
	CapturePointAction(blackboard) {
	effects = {WorldProp::BOMB_DEFUSED, true};
}

bool DODDefuseBombAction::findTargetLoc() {
	if (!CapturePointAction::findTargetLoc()) {
		return false;
	}
	targetRadius = 30.0f;
	return true;
}

bool DODDefuseBombAction::execute() {
	if (isDepleted()) {
		return true;
	}
	if (!GoToEntityAction::execute()) {
		return false;
	}
	if (!GoToAction::goalComplete()) {
		interruptable = true;
		return true;
	}
	interruptable = false;
	blackboard.getButtons().hold(IN_USE);
	Vector itemPos = UTIL_FindGround(
				item->GetCollideable()->GetCollisionOrigin());
	itemPos.z += HumanEyeHeight - 15.0f;
	if (blackboard.getSelf()->getEyesPos().z - itemPos.z > 10.0f) {
		blackboard.getButtons().hold(IN_DUCK);
	}
	blackboard.setViewTarget(itemPos);
	return false;
}

bool DODDefuseBombAction::isAvailable(edict_t* ent) {
	if (!CapturePointAction::isAvailable(ent)) {
		return false;
	}
	auto& players = blackboard.getPlayers();
	FOR_EACH_MAP_FAST(players, i) {
		if (players[i] != blackboard.getSelf()
				&& players[i]->getTeam() == blackboard.getSelf()->getTeam()
				&& players[i]->getCurrentPosition().DistTo(ent->GetCollideable()->GetCollisionOrigin()) < 100.0f) {
			if (DodPlayer(players[i]->getEdict()).isDefusing()) {
				return false;
			}
		}
	}
	return true;
}

bool DODDefuseBombAction::isAvailable(const DODObjective& obj) {
	return objectives->isDetonation()
			&& blackboard.getSelf()->getTeam() == obj.getOwner()
			&& obj.hasBombs()
			&& obj.hasBombTargetInState(DODObjective::BombState::ACTIVE);
}
