#include "DODDefuseBombAction.h"

#include <mods/dod/util/DODObjectiveResource.h>
#include <mods/dod/util/DodPlayer.h>
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <navmesh/nav.h>
#include <util/UtilTrace.h>
#include <in_buttons.h>

DODDefuseBombAction::DODDefuseBombAction(Blackboard& blackboard) :
		DODDefendPointAction(blackboard) {
	effects = {WorldProp::BOMB_DEFUSED, true};
}

bool DODDefuseBombAction::execute() {
	if (!GoToConsumableEntityAction::execute()) {
		return false;
	}
	if (!GoToAction::postCondCheck() || isDepleted()) {
		interruptable = true;
		return true;
	}
	interruptable = false;
	Vector itemPos = UTIL_FindGround(
			item->GetCollideable()->GetCollisionOrigin());
	itemPos.z += HumanHeight - 15.0f;
	blackboard.getButtons().hold(IN_USE);
	blackboard.setViewTarget(itemPos);
	return false;
}

bool DODDefuseBombAction::isAvailable(edict_t* ent) const {
	if (!CapturePointAction::isAvailable(ent)) {
		return false;
	}
	auto& players = blackboard.getPlayers();
	FOR_EACH_MAP_FAST(players, i) {
		if (players[i] != blackboard.getSelf()
				&& players[i]->getTeam() == blackboard.getSelf()->getTeam()
				&& players[i]->getCurrentPosition().DistTo(ent->GetCollideable()->GetCollisionOrigin()) < 100.0f) {
			DodPlayer teammate(players[i]->getEdict());
			if (isTeamMateActingOnBomb(teammate)) {
				return false;
			}
		}
	}
	return true;
}

bool DODDefuseBombAction::isTeamMateActingOnBomb(DodPlayer& teammate) const {
	return teammate.isDefusing();
}

bool DODDefuseBombAction::isAvailable(int idx) const {
	return isDetonationMap
			&& blackboard.getSelf()->getTeam()
					== objectiveResource->getOwner()[idx]
			&& objectiveResource->getNumBombsRequired()[idx] != 0
			&& isBombInState(idx, 2);
}
