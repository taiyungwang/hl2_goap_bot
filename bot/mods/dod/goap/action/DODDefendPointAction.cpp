#include "DODDefendPointAction.h"

#include <mods/dod/util/DODObjectiveResource.h>
#include <mods/dod/util/DODBombTarget.h>
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <util/EntityUtils.h>
#include <in_buttons.h>

 bool DODDefendPointAction::precondCheck() {
	 if (!GoToAction::precondCheck()) {
		 return false;
	 }
	 duration = 0;
	 return blackboard.getSelf()->getCurrentPosition().DistTo(targetLoc)
			 > targetRadius;
}

bool DODDefendPointAction::execute() {
	if (!GoToAction::execute()) {
		return false;
	}
	if (!GoToAction::postCondCheck()) {
		return true;
	}
	blackboard.lookStraight();
	blackboard.getButtons().hold(IN_DUCK);
	return duration++ < 2000;
}

bool DODDefendPointAction::isAvailable(int idx) {
	int owner = objectiveResource->getOwner()[idx];
	bool ours = owner == blackboard.getSelf()->getTeam();
	bool hasBombs = objectiveResource->getNumBombsRequired()[idx] > 0;
	// bomb map and not our bomb or already bombed.
	if ((isDetonationMap && hasBombs && (!ours || isBombInState(idx, 0)))
			// cap map and not our flag and enemy controlled
			|| (!isDetonationMap && !ours && owner > 0)) {
		enemyControlled++;
	}
	return ours && (!isDetonationMap ||(hasBombs && (isBombInState(idx, 1) || isBombInState(idx, 2))));
}

void DODDefendPointAction::selectFromActive(CUtlLinkedList<edict_t*>& active) {
	item = active.Count() == 1 && enemyControlled + 1 == items.Count()
			&& findNearestEntity(items, blackboard.getSelf()->getCurrentPosition()) == active[0]?
		active[0] : nullptr;
	enemyControlled = 0;
}

bool DODDefendPointAction::isBombInState(int idx, int state) const {
	auto& targets = capTarget[idx];
	FOR_EACH_VEC(targets, i) {
		if (DODBombTarget(targets[i]).getState() == state) {
			return true;
		}
	}
	return false;
}
