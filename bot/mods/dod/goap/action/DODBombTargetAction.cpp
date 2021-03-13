#include "DODBombTargetAction.h"

#include <mods/dod/player/DODObjectives.h>
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

bool DODBombTargetAction::isAvailable(int idx) {
	return objectives->isDetonation()
			&& blackboard.getSelf()->getTeam() != objectives->getOwner(idx)
			&& objectives->hasBombs(idx)
			&& objectives->isBombInState(idx, 1);
}
