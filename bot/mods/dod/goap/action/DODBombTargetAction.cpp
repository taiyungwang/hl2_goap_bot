#include "DODBombTargetAction.h"

#include <player/Blackboard.h>
#include <player/Player.h>
#include <mods/dod/util/DODObjectiveResource.h>

DODBombTargetAction::DODBombTargetAction(Blackboard& blackboard) :
DODDefuseBombAction(blackboard) {
	effects = {WorldProp::ALL_POINTS_CAPTURED, true};
	precond.Insert(WorldProp::HAS_BOMB, true);
}

bool DODBombTargetAction::isAvailable(int idx) const {
	return isDetonationMap
			&& blackboard.getSelf()->getTeam()
					!= objectiveResource->getOwner()[idx]
			&& objectiveResource->getNumBombsRequired()[idx] > 0
			&& isBombInState(idx, 1);
}
