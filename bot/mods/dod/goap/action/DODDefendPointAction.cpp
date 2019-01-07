#include "DODDefendPointAction.h"

#include <mods/dod/util/DODObjectiveResource.h>
#include <mods/dod/util/DODBombTarget.h>
#include <player/Blackboard.h>
#include <player/Player.h>

bool DODDefendPointAction::precondCheck() {
	duration = 0;
	return CapturePointAction::precondCheck();
}

bool DODDefendPointAction::isAvailable(int idx) const {
	return blackboard.getSelf()->getTeam()
					== objectiveResource->getOwner()[idx]
			&& (!isDetonationMap ||
					(objectiveResource->getNumBombsRequired()[idx] > 0
							&& (isBombInState(idx, 1) || isBombInState(idx, 2))));
}

bool DODDefendPointAction::isBombInState(int idx, int state) const {
	auto& targets = bombs[idx];
	FOR_EACH_VEC(targets, i) {
		if (DODBombTarget(targets[i]).getState() == state) {
			return true;
		}
	}
	return false;
}