#include "FilterSelfAndEnemies.h"

#include <player/Player.h>
#include <util/UtilTrace.h>
#include <eiface.h>

bool FilterSelfAndEnemies::ShouldHitEntity(IHandleEntity *pHandleEntity,
		int contentsMask) {
	if (pHandleEntity == self->GetIServerEntity()) {
		return false;
	}
	extern IVEngineServer* engine;
	extern CGlobalVars *gpGlobals;
	int idx = engine->IndexOfEdict(entityFromEntityHandle(const_cast<const IHandleEntity*>(pHandleEntity)));
	return (idx < 1 || idx > gpGlobals->maxClients)
			|| !Player::getPlayer(idx)->isEnemy(*Player::getPlayer(self));
}
