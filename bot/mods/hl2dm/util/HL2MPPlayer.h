#pragma once

#include <util/EntityInstance.h>

class HL2MPPlayer: public EntityInstance {
public:
	HL2MPPlayer(edict_t *ent) :
			EntityInstance(ent, "CHL2MP_Player") {
	}

	bool isOnLadder() {
		return classDef->getEntityVar("m_hLadder").getEntity(ent) != nullptr;
	}
};
