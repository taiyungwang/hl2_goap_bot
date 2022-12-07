#include "CapturePointAction.h"

#include <mods/dod/player/DODObjectives.h>
#include <mods/dod/player/DODObjective.h>
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <util/EntityUtils.h>
#include <nav_mesh/nav_entities.h>
#include <vstdlib/random.h>
#include <in_buttons.h>

CapturePointAction::CapturePointAction(Blackboard& blackboard) :
		GoToEntityWithGivenNameAction(blackboard, "dod_control_point") {
	precond[WorldProp::ROUND_STARTED] = true;
	effects = {WorldProp::ALL_POINTS_CAPTURED, true};
	allItemsVisible = true;
}

bool CapturePointAction::execute() {
	if (!GoToEntityWithGivenNameAction::execute()) {
		return false;
	}
	if (!GoToAction::goalComplete() || isDepleted()) {
		return true;
	}
	blackboard.getSelf()->lookStraight();
	blackboard.getButtons().hold(IN_DUCK);
	return false;
}

bool CapturePointAction::isAvailable(edict_t* ent) {
	const auto* obj = objectives->getObjective(ent);
	return obj != nullptr && isAvailable(*obj);
}

bool CapturePointAction::precondCheck() {
	if (!objectives->roundStarted()) {
		return false;
	}
	selectItem();
	if (item == nullptr) {
		return false;
	}
	setTargetLocAndRadius(objectives->getObjective(item)->getTargets()[0]);
	return true;
}

bool CapturePointAction::isAvailable(const DODObjective& obj) {
	return !objectives->isDetonation()
			&& obj.getOwner() != blackboard.getSelf()->getTeam();
}
