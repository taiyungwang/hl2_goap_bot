#pragma once

#include <voice/VoiceMessage.h>

class DODVoiceMessage: public VoiceMessage {
public:
	enum {
		ENEMY_AHEAD = VoiceMessage::COUNT,
		GRENADE,
		FIRE_IN_THE_HOLE,
		NEED_AMMO,
		SNIPER,
		MG_AHEAD,
		ROCKET_AHEAD
	};
};

