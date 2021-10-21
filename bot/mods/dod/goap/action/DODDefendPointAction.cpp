#include "DODDefendPointAction.h"

#include <mods/dod/player/DODObjectives.h>
#include <mods/dod/player/DODObjective.h>
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <player/HidingSpotSelector.h>
#include <util/EntityUtils.h>
#include <in_buttons.h>

bool DODDefendPointAction::execute() {
	return objectives->roundStarted() && isTargetValid() && SnipeAction::execute();
}

bool DODDefendPointAction::findTargetLoc() {
	if (!objectives->roundStarted()) {
		return false;
	}
	const auto &points = objectives->getCtrlPts();
	CUtlLinkedList<edict_t*> available;
	FOR_EACH_LL(points, i)
	{
		target = objectives->getObjective(points[i]);
		if (target != nullptr && isTargetValid()) {
			available.AddToTail(points[i]);
		}
	}
	while (available.Count() > 0) {
		edict_t *choice = randomChoice(available);
		available.FindAndRemove(choice);
		target = objectives->getObjective(choice);
		auto &spots = target->getHideSpots();
		// TODO: should we randomize spot selection?
		FOR_EACH_VEC(spots, i)
		{
			if (!selector->isInUse(spots[i], blackboard.getSelf()->getTeam())) {
				guardTarget = choice->GetCollideable()->GetCollisionOrigin();
				selectorId = spots[i];
				targetLoc = selector->getSpotPos(selectorId);
				if (target->hasBombTargetInState(DODObjective::BombState::ACTIVE)
						&& targetLoc.DistTo(guardTarget) < 300.0f) {
					selectorId = -1;
					continue;
				}
				selector->setInUse(selectorId, blackboard.getSelf()->getTeam(), true);
				return true;
			}
		}
	}
	return false;
}

bool DODDefendPointAction::isTargetValid() const {
	bool ours = target->getOwner() == blackboard.getSelf()->getTeam();
	return (ours && (!objectives->isDetonation() || (target->hasBombs() && target->hasBombTargetInState(DODObjective::BombState::AVAILABLE))))
			|| (!ours && target->hasBombTargetInState(DODObjective::BombState::ACTIVE));
}
