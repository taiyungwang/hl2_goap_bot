#include "FindCoverAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
#include <player/Vision.h>
#include <move/Navigator.h>
#include <edict.h>

FindCoverAction::FindCoverAction(Blackboard& blackboard) :
		GoToAction(blackboard) {
	effects = {WorldProp::ENEMY_SIGHTED, false};
}

bool FindCoverAction::ShouldSearch(CNavArea *adjArea, CNavArea *currentArea,
		float travelDistanceSoFar) {
	edict_t* target = getTarget();
	return ISearchSurroundingAreasFunctor::ShouldSearch(adjArea, currentArea, travelDistanceSoFar)
			&& target != nullptr && Navigator::getArea(target) != currentArea;
}

bool FindCoverAction::operator() ( CNavArea *area, CNavArea *priorArea, float travelDistanceSoFar ) {
	edict_t* target = getTarget();
	bool isVisible = currentArea == area
			|| (area->IsPotentiallyVisible(Navigator::getArea(target))
			&& UTIL_IsVisible(area->GetCenter(), blackboard, target));
	if (!isVisible) {
		this->hideArea = area;
	}
	return isVisible;
}

bool FindCoverAction::onPlanningFinished() {
	if(!GoToAction::onPlanningFinished()) {
		return false;
	}
	CNavArea* targetArea = Navigator::getArea(getTarget());
	for (int i = 0; i < path.Count(); i++)  {
		if (path[i] == targetArea) {
			return false;
		}
	}
	return true;
}

bool FindCoverAction::findTargetLoc() {
	edict_t* target = getTarget();
	if (target == nullptr) {
		return false;
	}
	this->hideArea = nullptr;
	currentArea = Navigator::getArea(blackboard.getSelf()->getEdict());
	SearchSurroundingAreas(currentArea, *this);
	return hideArea != nullptr;
}

void FindCoverAction::PostSearch( void ) {
	if (hideArea != nullptr) {
		targetLoc = hideArea->GetCenter();
	}
}

edict_t* FindCoverAction::getTarget() const {
	auto targetPlayer = blackboard.getTargetedPlayer();
	return targetPlayer == nullptr || targetPlayer->isDead() ? nullptr : targetPlayer->getEdict();
}


