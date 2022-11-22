#include "GiveAmmoAction.h"
#include <mods/dod/voice/DODVoiceMessage.h>
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <voice/VoiceMessageSender.h>
#include <voice/AffirmativeVoiceMessage.h>

GiveAmmoAction::GiveAmmoAction(Blackboard &blackboard,
		CommandHandler &commandHandler) :
		GoToEntityAction(blackboard), Receiver(commandHandler) {
	effects = { WorldProp::HEARD_NEED_AMMO, false };
	precond[WorldProp::ENEMY_SIGHTED] = false;
}

bool GiveAmmoAction::init() {
	if (!GoToEntityAction::init()) {
		return false;
	}
	if (item != nullptr) {
		blackboard.getSelf()->getVoiceMessageSender().sendMessage(
				std::make_shared<AffirmativeVoiceMessage>(
						blackboard.getSelf()->getEdict()));
	}
	item = nullptr;
	return true;
}

bool GiveAmmoAction::goalComplete() {
	if (!GoToAction::goalComplete()) {
		return false;
	}
	extern IServerPluginHelpers *helpers;
	helpers->ClientCommand(blackboard.getSelf()->getEdict(), "dropammo");
	return true;
}

bool GiveAmmoAction::receive(edict_t *sender, const CCommand &command) {
	Player *player = Player::getPlayer(sender);
	Bot *self = blackboard.getSelf();
	if (player->getTeam() == self->getTeam()
			&& blackboard.getSelf()->getCurrentPosition().DistTo(
					player->getCurrentPosition()) < 500.0f
			&& self->getVoiceMessageSender().isMessage<DODVoiceMessage::NeedAmmo>(command.Arg(0))
			&& self->canSee(*player)) {
		item = sender;
		self->setResetPlanner(true);
		return true;
	}
	return false;
}
