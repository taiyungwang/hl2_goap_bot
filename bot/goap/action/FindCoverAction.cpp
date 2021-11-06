#include "FindCoverAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
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
	Bot* self = blackboard.getSelf();
	if (currentArea != area
			&& !area->IsPotentiallyVisible(Navigator::getArea(target, self->getTeam()))
			&& !self->canShoot(eyes, target)) {
		this->hideArea = area;
		return false;
	}
	return true;
}

bool FindCoverAction::onPlanningFinished() {
	return getTarget() != nullptr;
}

bool FindCoverAction::findTargetLoc() {
	edict_t *target = getTarget();
	if (target == nullptr) {
		return false;
	}
	this->hideArea = nullptr;
	auto self = blackboard.getSelf();
	currentArea = Navigator::getArea(self->getEdict(), self->getTeam());
	SearchSurroundingAreas(currentArea, *this);
	return hideArea != nullptr;
}

void FindCoverAction::PostSearch(void) {
	if (hideArea != nullptr) {
		targetLoc = hideArea->GetCenter();
	}
}

edict_t* FindCoverAction::getTarget() const {
	auto targetPlayer = Player::getPlayer(blackboard.getSelf()->getVision().getTargetedPlayer());
	return targetPlayer == nullptr || !targetPlayer->isInGame() ? nullptr : targetPlayer->getEdict();
}

