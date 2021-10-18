#pragma once

#include "NeedBackupVoiceMessage.h"

class AreaClearVoiceMessage: public NeedBackupVoiceMessage {
public:
	AreaClearVoiceMessage(edict_t *self) :
		NeedBackupVoiceMessage(self) {
	}

private:
	bool checkShouldSend(const VoiceMessage *other) const override;
};

