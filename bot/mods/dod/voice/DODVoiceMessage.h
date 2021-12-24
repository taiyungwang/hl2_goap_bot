#pragma once

#include <voice/AreaClearVoiceMessage.h>


#define DECL_ENEMY_AHEAD_VOICE_MSG_CLASS(name)\
class name: public AreaClearVoiceMessage {\
public:\
	name(edict_t* self): AreaClearVoiceMessage(self) {\
	}\
}

namespace DODVoiceMessage {
class FireInTheHole: public VoiceMessage {
public:
	FireInTheHole(edict_t *sender) :
			VoiceMessage(sender) {
	}
};

class NeedAmmo: public VoiceMessage {
public:
	NeedAmmo(edict_t *sender) :
			VoiceMessage(sender) {
	}
};

DECL_ENEMY_AHEAD_VOICE_MSG_CLASS(SniperAheadVoiceMessage);

DECL_ENEMY_AHEAD_VOICE_MSG_CLASS(MGAheadVoiceMessage);

DECL_ENEMY_AHEAD_VOICE_MSG_CLASS(RocketAheadVoiceMessage);

}
