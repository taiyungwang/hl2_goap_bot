#pragma once

struct edict_t;

class VoiceMessage {
public:

	VoiceMessage(edict_t *self) :
		sender(self) {
	}

	virtual ~VoiceMessage() {
	}

	virtual bool operator()(const VoiceMessage *other) {
		return checkShouldSend(other);
	}

	bool canSend() const {
		return send;
	}

	edict_t* getSender() const {
		return sender;
	}

protected:
	edict_t *sender;

	bool send = true;

	virtual bool checkShouldSend(const VoiceMessage *other) const {
		return true;
	}
};
