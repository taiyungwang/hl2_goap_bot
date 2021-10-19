#include "DODBombTargetAction.h"

#include <mods/dod/player/DODObjectives.h>
#include <mods/dod/player/DODObjective.h>
#include <mods/dod/util/DodPlayer.h>
#include <player/Blackboard.h>
#include <player/Bot.h>

DODBombTargetAction::DODBombTargetAction(Blackboard &blackboard) :
		DODDefuseBombAction(blackboard) {
	effects = { WorldProp::ALL_POINTS_CAPTURED, true };
	precond.Insert(WorldProp::HAS_BOMB, true);
}

bool DODBombTargetAction::isAvailable(const DODObjective &obj) {
	return objectives->isDetonation()
			&& blackboard.getSelf()->getTeam() != obj.getOwner()
			&& obj.hasBombs() && obj.hasBombTargetInState(DODObjective::BombState::AVAILABLE);
}

bool DODBombTargetAction::isTeammateActing(edict_t* teammate) const {
	return DodPlayer(teammate).isPlanting();
}
