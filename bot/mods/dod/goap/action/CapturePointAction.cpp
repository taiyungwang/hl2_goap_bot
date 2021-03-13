#include "CapturePointAction.h"

#include <mods/dod/player/DODObjectives.h>
#include <event/EventInfo.h>
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <util/EntityUtils.h>
#include <nav_mesh/nav_entities.h>
#include <vstdlib/random.h>
#include <in_buttons.h>

CapturePointAction::CapturePointAction(Blackboard& blackboard) :
		GoToConsumableEntityAction(blackboard, "dod_control_point") {
	allItemsVisible = true;
	precond.Insert(WorldProp::ROUND_STARTED, true);
	effects = {WorldProp::ALL_POINTS_CAPTURED, true};
}

bool CapturePointAction::execute() {
	if (!GoToConsumableEntityAction::execute()) {
		return false;
	}
	if (!GoToAction::postCondCheck() || isDepleted()) {
		return true;
	}
	blackboard.lookStraight();
	blackboard.getButtons().hold(IN_DUCK);
	return false;
}

bool CapturePointAction::isAvailable(edict_t* ent) {
	int idx = objectives->getIndex(ent);
	return idx != -1 && isAvailable(idx);
}

bool CapturePointAction::findTargetLoc() {
	if (!objectives->roundStarted()) {
		return false;
	}
	selectItem();
	if (item == nullptr) {
		return false;
	}
	setTargetLocAndRadius(objectives->getCapTarget(objectives->getIndex(item))[0]);
	return true;
}

bool CapturePointAction::isAvailable(int idx) {
	return !objectives->isDetonation()
			&& objectives->getOwner(idx) != blackboard.getSelf()->getTeam();
}

void CapturePointAction::selectFromActive(CUtlLinkedList<edict_t*>& active) {
	if (active.Count() == 0) {
		return;
	}
	item = active[active.Tail()];
	if (active.Count() == 1) {
		return;
	}
	float totalDist = 0.0f;
	CUtlLinkedList<float> prob;
	FOR_EACH_LL(active, i) {
		prob.AddToTail(1.0f / active[i]->GetCollideable()->GetCollisionOrigin().DistTo(blackboard.getSelf()->getCurrentPosition()));
		totalDist += prob[prob.Tail()];
	}
	float totalProb = 0.0f;
	FOR_EACH_LL(prob, i) {
		prob[i] /= totalDist;
		if (i > 0) {
			prob[i] += prob[i - 1];
		}
	}
	float choice = RandomFloat(0, 1.0f);
	FOR_EACH_LL(prob, i) {
		if (choice < prob[i]) {
			item = active[i];
			break;
		}
	}
}
