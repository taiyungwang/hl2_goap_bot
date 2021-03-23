#include "MoveTraceFilter.h"

#include <player/Player.h>
#include <util/BasePlayer.h>
#include <eiface.h>
#include <iplayerinfo.h>

MoveTraceFilter::MoveTraceFilter(const Player &self, edict_t *target) :
		team(self.getTeam()) {
	add(self.getEdict());
	add(BasePlayer(self.getEdict()).getGroundEntity());
	add(target);
}

bool MoveTraceFilter::ShouldHitEntity(IHandleEntity *pHandleEntity,
		int contentsMask) {
	if (FilterList::ShouldHitEntity(pHandleEntity, contentsMask)) {
		extern IPlayerInfoManager *playerinfomanager;
		extern IVEngineServer *engine;
		if (team == 0) {
			return true;
		}
		extern CGlobalVars *gpGlobals;
		int index = pHandleEntity->GetRefEHandle().GetEntryIndex();
		return index >= gpGlobals->maxClients
				|| team
						!= playerinfomanager->GetPlayerInfo(
								engine->PEntityOfEntIndex(index))->GetTeamIndex();
	}
	return false;
}
