#include "NeedBackupVoiceMessage.h"

#include <typeinfo>

bool NeedBackupVoiceMessage::operator()(const VoiceMessage *msg) {
	if (typeid(*this) == typeid(*msg) && !checkShouldSend(msg)) {
		send = false;
		return true;
	}
	return false;
}
