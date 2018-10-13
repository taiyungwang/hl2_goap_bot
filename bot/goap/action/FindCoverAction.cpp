#include "FindCoverAction.h"

#include <player/Blackboard.h>
#include <player/Player.h>
#include <player/Vision.h>
#include <edict.h>

FindCoverAction::FindCoverAction(Blackboard& blackboard) :
		FindPathAction(blackboard) {
	precond.Insert(WorldProp::AT_LOCATION, true);
	effects = {WorldProp::ENEMY_SIGHTED, false};
}

bool FindCoverAction::operator() ( CNavArea *area, CNavArea *priorArea, float travelDistanceSoFar ) {
	edict_t* target = getTarget();
	bool isVisible = area->IsPotentiallyVisible(getArea(target))
			&& UTIL_IsVisible(area->GetCenter(),blackboard, target);
	if (!isVisible) {
		this->hideArea = area;
	}
	return isVisible;
}

bool FindCoverAction::precondCheck() {
	edict_t* target = getTarget();
	if (target == nullptr) {
		return false;
	}
	this->hideArea = nullptr;
	CNavArea* area = getArea(blackboard.getSelf()->getEdict());
	SearchSurroundingAreas(area, *this);
	if (hideArea == nullptr) {
		return false;
	}
	return true;
}

void FindCoverAction::PostSearch( void ) {
	if (hideArea != nullptr) {
		targetLoc = hideArea->GetCenter();
		build();
	}
}

bool FindCoverAction::postCondCheck() {
	return !UTIL_IsVisible(blackboard.getSelf()->getEyesPos(), blackboard, getTarget());
}

edict_t* FindCoverAction::getTarget() const {
	auto targetPlayer = blackboard.getTargetedPlayer();
	return targetPlayer == nullptr || targetPlayer->isDead() ? nullptr : targetPlayer->getEdict();
}


