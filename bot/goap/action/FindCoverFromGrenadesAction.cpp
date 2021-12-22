#include "FindCoverFromGrenadesAction.h"

#include <move/Navigator.h>
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <player/FilterSelfAndEnemies.h>
#include <util/BaseGrenade.h>
#include <util/UtilTrace.h>
#include <eiface.h>

FindCoverFromGrenadesAction::FindCoverFromGrenadesAction(Blackboard &blackboard) :
		FindCoverAction(blackboard) {
	precond.clear();
	effects = { WorldProp::EXPLOSIVE_NEAR, false };
}

void FindCoverFromGrenadesAction::setAvoidAreas() {
	// TODO: this assumes that only relevant grenades are in the visible entities list.
	maxRange = 0.0f;
	for (auto i: blackboard.getSelf()->getVision().getVisibleEntities()) {
		extern IVEngineServer *engine;
		edict_t *entity = engine->PEntityOfEntIndex(i);
		float range = BaseGrenade(entity).getDmgRadius() + HalfHumanWidth;
		if (range > maxRange) {
			maxRange = range;
		}
		if (entity == nullptr || entity->IsFree()) {
			continue;
		}
		CNavArea* area = Navigator::getArea(entity, blackboard.getSelf()->getTeam());
		if (area != nullptr) {
			areasToAvoid[area] = entity;
		}
	}
}

void FindCoverFromGrenadesAction::getAvoidPosition(Vector& pos, edict_t *avoid) const {
	pos = avoid->GetCollideable()->GetCollisionOrigin();
}

bool FindCoverFromGrenadesAction::waitInCover() const {
	for (auto avoid: areasToAvoid) {
		if (!std::get<1>(avoid)->IsFree()) {
			return true;
		}
	}
	return false;
}
