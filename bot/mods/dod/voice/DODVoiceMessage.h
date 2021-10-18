#pragma once

#include <voice/VoiceMessage.h>

namespace DODVoiceMessage {
class FireInTheHole: public VoiceMessage {
public:
	FireInTheHole(edict_t *sender) :
			VoiceMessage(sender) {
	}
};

}
