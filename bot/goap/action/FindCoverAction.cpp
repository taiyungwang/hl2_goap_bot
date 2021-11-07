#include "FindCoverAction.h"

#include <player/Blackboard.h>
#include <player/FilterSelfAndEnemies.h>
#include <player/Bot.h>
#include <player/Vision.h>
#include <move/Navigator.h>
#include <util/UtilTrace.h>

FindCoverAction::FindCoverAction(Blackboard& blackboard) :
		GoToAction(blackboard) {
	effects = {WorldProp::ENEMY_SIGHTED, false};
	precond.Insert(WorldProp::OUT_OF_AMMO, false);
	sprint = true;
}

bool FindCoverAction::ShouldSearch(CNavArea *adjArea, CNavArea *currentArea,
		float travelDistanceSoFar) {
	int team = blackboard.getSelf()->getTeam();
	if (team == 0) {
		team = TEAM_ANY;
	}
	return !adjArea->IsBlocked(team)
			&& enemyAreas.find(adjArea) == enemyAreas.end();
}

bool FindCoverAction::operator() (CNavArea *area, CNavArea *priorArea, float travelDistanceSoFar) {
	Vector eyes(area->GetCenter());
	eyes.z += HumanEyeHeight;
	Bot* self = blackboard.getSelf();
	FilterSelfAndEnemies filter(self->getEdict());
	trace_t result;
	for (CNavArea* enemyArea: enemyAreas) {
		if (area->IsPotentiallyVisible(enemyArea)) {
			return true;
		}
		Vector enemyEyes(enemyArea->GetCenter());
		enemyEyes.z += HumanEyeHeight;
		UTIL_TraceLine(eyes, enemyEyes, MASK_SHOT, &filter, &result);
		if (!result.DidHit()) {
			return true;
		}
	}
	this->hideArea = area;
	return false;
}

bool FindCoverAction::execute() {
	return blackboard.getSelf()->getVision().getVisibleEnemies().empty()
			|| GoToAction::execute();
}

bool FindCoverAction::findTargetLoc() {
	this->hideArea = nullptr;
	auto self = blackboard.getSelf();
	currentArea = Navigator::getArea(self);
	enemyAreas.clear();
	for (auto i: blackboard.getSelf()->getVision().getVisibleEnemies()) {
		CNavArea* area = Navigator::getArea(Player::getPlayer(i));
		if (area != nullptr) {
			enemyAreas.insert(area);
		}
	}
	SearchSurroundingAreas(currentArea, *this);
	return hideArea != nullptr;
}

void FindCoverAction::PostSearch(void) {
	if (hideArea != nullptr) {
		targetLoc = hideArea->GetCenter();
	}
}


