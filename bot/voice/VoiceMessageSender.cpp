#include "VoiceMessageSender.h"

#include "VoiceMessage.h"
#include <player/Bot.h>
#include <player/Vision.h>
#include <eiface.h>

bool VoiceMessageSender::sendMessage(const std::shared_ptr<VoiceMessage> &message) {
	if (dynamic_cast<Bot*>(Player::getPlayer(message->getSender()))->getVision().getNearbyTeammates().empty()) {
		return false;
	}
	extern IVEngineServer *engine;
	float time = engine->Time();
	while (!sentMessages.empty()
			&& time - std::get<0>(sentMessages.front()) > 10.0f) {
		sentMessages.pop_front();
	}
	SentMessage newMsg = std::make_tuple(time, message);
	for (auto sent : sentMessages) {
		if (std::get<1>(newMsg)->operator()(std::get<1>(sent).get())) {
			break;
		}
	}
	const auto &msgType = typeid(*message);
	if (messages.find(msgType) != messages.end()
			&& std::get<1>(newMsg)->canSend()) {
		extern IServerPluginHelpers *helpers;
		helpers->ClientCommand(message->getSender(),
				messages.at(msgType).c_str());
		sentMessages.push_back(newMsg);
		return true;
	}
	return false;
}
