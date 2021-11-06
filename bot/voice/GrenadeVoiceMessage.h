#pragma once

#include "AreaClearVoiceMessage.h"

class GrenadeVoiceMessage: public AreaClearVoiceMessage {
public:
	GrenadeVoiceMessage(edict_t* self): AreaClearVoiceMessage(self) {
	}
};
