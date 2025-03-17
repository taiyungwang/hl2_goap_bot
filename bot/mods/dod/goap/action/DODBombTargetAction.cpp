#include "DODBombTargetAction.h"

#include <mods/dod/player/DODObjectives.h>
#include <mods/dod/player/DODObjective.h>
#include <mods/dod/util/DodPlayer.h>
#include <player/Bot.h>

DODBombTargetAction::DODBombTargetAction(Bot *self) :
		DODDefuseBombAction(self) {
	effects = { WorldProp::ALL_POINTS_CAPTURED, true };
	precond[WorldProp::HAS_BOMB] = true;
}

bool DODBombTargetAction::isAvailable(const DODObjective &obj) {
	return objectives->isDetonation()
			&& self->getTeam() != obj.getOwner()
			&& obj.hasBombs() && obj.hasBombTargetInState(DODObjective::BombState::AVAILABLE);
}

bool DODBombTargetAction::isActingOnBomb(edict_t* teammate) const {
	return DodPlayer(teammate).isPlanting();
}
