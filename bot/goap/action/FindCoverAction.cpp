#include "FindCoverAction.h"

#include <player/Blackboard.h>
#include <player/FilterSelfAndEnemies.h>
#include <player/Bot.h>
#include <player/Vision.h>
#include <move/Navigator.h>
#include <nav_mesh/nav_area.h>

FindCoverAction::FindCoverAction(Blackboard& blackboard) :
		GoToAction(blackboard), NavMeshPathBuilder(blackboard.getSelf()->getTeam()) {
	effects = {WorldProp::MULTIPLE_ENEMY_SIGHTED, false};
	// this is needed so that hl2dm bots aren't trying to find cover when their weapon is out of ammo.
	precond[WorldProp::OUT_OF_AMMO] = false;
	sprint = true;
}

bool FindCoverAction::foundGoal(CNavArea *area) {
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
			return false;
		}
		Vector enemyEyes;
		getAvoidPosition(enemyEyes, std::get<1>(avoid));
		trace_t result;
		Bot::canSee(result, eyes, enemyEyes);
		if (!result.DidHit()) {
			return false;
		}
	}
	targetLoc = area->GetCenter();
	return true;
}

void FindCoverAction::setAvoidAreas() {
	for (auto i: blackboard.getSelf()->getVision().getVisibleEnemies()) {
		const Player* player = Player::getPlayer(i);
		CNavArea* area = player->getArea();
		if (area != nullptr) {
			areasToAvoid[area] = player->getEdict();
		}
	}
}

void FindCoverAction::getAvoidPosition(Vector& pos, edict_t *avoid) const {
	pos = Player::getPlayer(avoid)->getEyesPos();
}

bool FindCoverAction::precondCheck() {
	areasToAvoid.clear();
	setAvoidAreas();
	if (areasToAvoid.empty()) {
		return false;
	}
	auto self = blackboard.getSelf();
	NavMeshPathBuilder::Path path;
	build(path, self->getArea());
	if (path.empty()) {
		return false;
	}
	self->getNavigator()->getPath().swap(path);
	self->getNavigator()->start(targetLoc, targetRadius, sprint);
	return true;
}

float FindCoverAction::getHeuristicCost(CNavArea *area) const {
	float cost = 0.0f;
	Vector eyes(area->GetCenter());
	for (auto avoid: areasToAvoid) {
		Vector enemyEyes;
		getAvoidPosition(enemyEyes, std::get<1>(avoid));
		trace_t result;
		Bot::canSee(result, eyes, enemyEyes);
		if (result.DidHit()) {
			continue;
		}
		cost -= area->GetCenter().DistTo(std::get<1>(avoid)->GetCollideable()->GetCollisionOrigin());
	}
	for (auto i: blackboard.getSelf()->getVision().getNearbyTeammates()) {
		cost += area->GetCenter().DistTo(Player::getPlayer(i)->getCurrentPosition());
	}
	return MAX(0.0, cost);
}

bool FindCoverAction::shouldSearch(CNavArea *area) const {
	return NavMeshPathBuilder::shouldSearch(area)
			&& areasToAvoid.find(area) == areasToAvoid.end();
}
