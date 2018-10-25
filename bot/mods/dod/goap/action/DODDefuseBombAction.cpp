#include "DODDefuseBombAction.h"

#include <mods/dod/util/DODObjectiveResource.h>
#include <player/Blackboard.h>
#include <player/Player.h>
#include <navmesh/nav.h>
#include <util/UtilTrace.h>
#include <in_buttons.h>

DODDefuseBombAction::DODDefuseBombAction(Blackboard& blackboard) :
		DODDefendPointAction(blackboard) {
	effects = {WorldProp::BOMB_DEFUSED, true};
	targetRadius = 40.0f;
}

bool DODDefuseBombAction::execute() {
	if (!GoToConsumableEntityAction::execute()) {
		return false;
	}
	if (!GoToAction::postCondCheck() || isDepleted()) {
		return true;
	}
	Vector itemPos = UTIL_FindGround(
			item->GetCollideable()->GetCollisionOrigin());
	itemPos.z += HumanHeight - 15.0f;
	if (blackboard.getAimAccuracy(itemPos) > 0.999f) {
		blackboard.getButtons().hold(IN_USE);
	}
	blackboard.setViewTarget(itemPos);
	return false;
}

bool DODDefuseBombAction::isAvailable(int idx) const {
	return isDetonationMap
			&& blackboard.getSelf()->getTeam()
					== objectiveResource->getOwner()[idx]
			&& objectiveResource->getNumBombsRequired()[idx] != 0
			&& isBombInState(idx, 2);
}
