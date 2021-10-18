#pragma once

#include "VoiceMessage.h"

class NeedBackupVoiceMessage: public VoiceMessage {
public:
	NeedBackupVoiceMessage(edict_t *self) :
			VoiceMessage(self) {
	}

	virtual ~NeedBackupVoiceMessage() {
	}

	bool operator()(const VoiceMessage *msg)
			override;

protected:
	virtual bool checkShouldSend(const VoiceMessage* msg) const override {
		return msg->getSender() != sender;
	}
};
