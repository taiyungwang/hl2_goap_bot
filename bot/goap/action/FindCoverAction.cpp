#include "FindCoverAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
#include <player/Vision.h>
#include <move/Navigator.h>
#include <edict.h>

FindCoverAction::FindCoverAction(Blackboard& blackboard) :
		GoToAction(blackboard) {
	effects = {WorldProp::ENEMY_SIGHTED, false};
	precond.Insert(WorldProp::OUT_OF_AMMO, false);
}

bool FindCoverAction::ShouldSearch(CNavArea *adjArea, CNavArea *currentArea,
		float travelDistanceSoFar) {
	edict_t* target = getTarget();
	return ISearchSurroundingAreasFunctor::ShouldSearch(adjArea, currentArea, travelDistanceSoFar)
			&& target != nullptr && Navigator::getArea(target, blackboard.getSelf()->getTeam()) != currentArea;
}

bool FindCoverAction::operator() (CNavArea *area, CNavArea *priorArea, float travelDistanceSoFar) {
	edict_t* target = getTarget();
	Vector eyes(area->GetCenter());
	eyes.z += HumanEyeHeight;
	if (currentArea != area
			&& !area->IsPotentiallyVisible(Navigator::getArea(target, blackboard.getSelf()->getTeam()))
			&& !UTIL_IsVisible(eyes, blackboard, target)) {
		this->hideArea = area;
		return true;
	}
	return false;
}

float FindCoverAction::getCost() {
	return getTarget() == nullptr || (findTargetLoc()
			&& blackboard.getNavigator()->buildPath(targetLoc, path)) ?
			path.Count() : INFINITY;
}

bool FindCoverAction::onPlanningFinished() {
	edict_t *target = getTarget();
	if (target == nullptr) {
		return false;
	}
	CNavArea *targetArea = Navigator::getArea(target,
			blackboard.getSelf()->getTeam());
	for (int i = 0; i < path.Count(); i++) {
		if (path[i] == targetArea) {
			return false;
		}
	}
	return true;
}

bool FindCoverAction::findTargetLoc() {
	edict_t *target = getTarget();
	if (target == nullptr) {
		return false;
	}
	this->hideArea = nullptr;
	currentArea = Navigator::getArea(blackboard.getSelf()->getEdict(), blackboard.getSelf()->getTeam());
	SearchSurroundingAreas(currentArea, *this);
	return hideArea != nullptr;
}

void FindCoverAction::PostSearch(void) {
	if (hideArea != nullptr) {
		targetLoc = hideArea->GetCenter();
	}
}

edict_t* FindCoverAction::getTarget() const {
	auto targetPlayer = blackboard.getTargetedPlayer();
	return targetPlayer == nullptr || targetPlayer->isDead() ? nullptr : targetPlayer->getEdict();
}


