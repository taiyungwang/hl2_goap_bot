#pragma once

#include <util/BasePlayer.h>

class HL2MPPlayer: public BasePlayer {
public:
	HL2MPPlayer(edict_t *ent): BasePlayer("CHL2MP_Player", ent) {
	}

	bool isOnLadder() {
		return classDef->getEntityVar("m_hLadder").getEntity(ent) != nullptr;
	}
};
