#pragma once

#include "NeedBackupVoiceMessage.h"

class AffirmativeVoiceMessage: public NeedBackupVoiceMessage {
public:
	AffirmativeVoiceMessage(edict_t *self) :
			NeedBackupVoiceMessage(self) {
	}
};
