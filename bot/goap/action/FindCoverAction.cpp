#include "FindCoverAction.h"

#include <player/Blackboard.h>
#include <player/FilterSelfAndEnemies.h>
#include <player/Bot.h>
#include <player/Vision.h>
#include <move/Navigator.h>

FindCoverAction::FindCoverAction(Blackboard& blackboard) :
		GoToAction(blackboard) {
	effects = {WorldProp::MULTIPLE_ENEMY_SIGHTED, false};
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
			&& areasToAvoid.find(adjArea) == areasToAvoid.end();
}

bool FindCoverAction::operator() (CNavArea *area, CNavArea *priorArea, float travelDistanceSoFar) {
	if (area == startArea) {
		return true;
	}
	Vector eyes(area->GetCenter());
	eyes.z += HumanEyeHeight;
	Bot* self = blackboard.getSelf();
	FilterSelfAndEnemies filter(self->getEdict());
	trace_t result;
	for (auto avoid: areasToAvoid) {
		if (area->GetCenter().DistTo(std::get<1>(avoid)->GetCollideable()->GetCollisionOrigin()) > maxRange) {
			continue;
		}
		if (area->IsPotentiallyVisible(std::get<0>(avoid))) {
			return true;
		}
		Vector enemyEyes;
		getAvoidPosition(enemyEyes, std::get<1>(avoid));
		if (Bot::canSee(eyes, enemyEyes)) {
			return true;
		}
	}
	this->hideArea = area;
	return false;
}

void FindCoverAction::setAvoidAreas() {
	for (auto i: blackboard.getSelf()->getVision().getVisibleEnemies()) {
		const Player* player = Player::getPlayer(i);
		CNavArea* area = Navigator::getArea(player);
		if (area != nullptr) {
			areasToAvoid[area] = player->getEdict();;
		}
	}
}

void FindCoverAction::getAvoidPosition(Vector& pos, edict_t *avoid) const {
	pos = Player::getPlayer(avoid)->getEyesPos();
}

bool FindCoverAction::findTargetLoc() {
	this->hideArea = nullptr;
	areasToAvoid.clear();
	setAvoidAreas();
	if (areasToAvoid.empty()) {
		return false;
	}
	startArea = Navigator::getArea(blackboard.getSelf());
	if (startArea == nullptr) {
		return false;
	}
	SearchSurroundingAreas(startArea, *this);
	return hideArea != nullptr;
}

void FindCoverAction::PostSearch(void) {
	if (hideArea != nullptr) {
		targetLoc = hideArea->GetCenter();
	}
}


