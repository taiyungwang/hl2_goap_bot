#include "AreaClearVoiceMessage.h"

#include <player/Player.h>

bool AreaClearVoiceMessage::checkShouldSend(const VoiceMessage *other) const {
	Player *lastSender = Player::getPlayer(other->getSender()), *newSender =
			Player::getPlayer(sender);
	// TODO: check to see if the person speaking is visible?
	return NeedBackupVoiceMessage::checkShouldSend(other)
			&& ((lastSender->getTeam() != newSender->getTeam()
					|| lastSender->getCurrentPosition().DistTo(
							newSender->getCurrentPosition()) >= 1000.0f));
}
