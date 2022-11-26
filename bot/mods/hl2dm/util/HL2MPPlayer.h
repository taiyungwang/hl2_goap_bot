#pragma once

#include <util/BasePlayer.h>

class HL2MPPlayer: public BasePlayer {
public:
	HL2MPPlayer(edict_t *ent): BasePlayer(ent) {
	}

	bool isOnLadder() const {
		return getEntity("m_hLadder") != nullptr;
	}
};
