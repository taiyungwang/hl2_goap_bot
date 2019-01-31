#include "DODBombTargetAction.h"

#include <mods/dod/util/DODObjectiveResource.h>
#include <mods/dod/util/DodPlayer.h>
#include <player/Blackboard.h>
#include <player/Bot.h>

DODBombTargetAction::DODBombTargetAction(Blackboard& blackboard) :
DODDefuseBombAction(blackboard) {
	effects = {WorldProp::ALL_POINTS_CAPTURED, true};
	precond.Insert(WorldProp::HAS_BOMB, true);
}

bool DODBombTargetAction::execute() {
	bool planted = DODDefuseBombAction::execute();
	if (planted) {
		blackboard.lookStraight();
	}
	return planted && (!GoToAction::postCondCheck() || duration++ >= 1000);
}

bool DODBombTargetAction::isAvailable(int idx) const {
	return isDetonationMap
			&& blackboard.getSelf()->getTeam()
					!= objectiveResource->getOwner()[idx]
			&& objectiveResource->getNumBombsRequired()[idx] > 0
			&& isBombInState(idx, 1);
}

bool DODBombTargetAction::isTeamMateActingOnBomb(DodPlayer& teammate) const {
	return teammate.isPlanting();
}
