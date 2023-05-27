#include "DODDefendPointAction.h"

#include <mods/dod/player/DODObjectives.h>
#include <mods/dod/player/DODObjective.h>
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <player/HidingSpotSelector.h>
#include <util/EntityUtils.h>
#include <in_buttons.h>


static ConVar dodDefendChance("mybot_dod_defend_chance", "0.2");

DODDefendPointAction::DODDefendPointAction(Blackboard &blackboard) :
		SnipeAction(blackboard) {
	precond[WorldProp::ROUND_STARTED] = true;
	effects = { WorldProp::POINTS_DEFENDED, true };
}

bool DODDefendPointAction::precondCheck() {
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
		for (auto spot: spots)
		{
			if (!selector->isInUse(spot, blackboard.getSelf()->getTeam())) {
				guardTarget = choice->GetCollideable()->GetCollisionOrigin();
				selectorId = spot;
				targetLoc = selector->getSpotPos(selectorId);
				if (objectives->isDetonation()
						&& target->hasBombTargetInState(DODObjective::BombState::ACTIVE)
						&& targetLoc.DistTo(guardTarget) < 300.0f) {
					selectorId = -1;
					continue;
				}
				selector->setInUse(selectorId, blackboard.getSelf()->getTeam(),
						true);
				return true;
			}
		}
	}
	return false;
}

float DODDefendPointAction::getChanceToExec() const {
	return dodDefendChance.GetFloat();
}

bool DODDefendPointAction::isTargetValid() const {
	bool ours = target->getOwner() == blackboard.getSelf()->getTeam();
	if (!objectives->isDetonation()) {
		return !ours;
	}
	if (!target->hasBombs()) {
		return false;
	}
	return ours ?
			target->hasBombTargetInState(DODObjective::BombState::AVAILABLE) :
			target->hasBombTargetInState(DODObjective::BombState::ACTIVE);
}
