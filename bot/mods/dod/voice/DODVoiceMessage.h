#pragma once

#include <voice/VoiceMessage.h>

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
}
