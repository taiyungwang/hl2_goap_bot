#include "GiveAmmoAction.h"
#include <mods/dod/voice/DODVoiceMessage.h>
#include <player/Bot.h>

GiveAmmoAction::GiveAmmoAction(Bot *self,
		CommandHandler &commandHandler) :
		GoToEntityAction(self), Receiver(commandHandler) {
	effects = { WorldProp::HEARD_NEED_AMMO, false };
	precond[WorldProp::ENEMY_SIGHTED] = false;
}

bool GiveAmmoAction::init() {
	if (!GoToEntityAction::init()) {
		return false;
	}
	if (item != nullptr) {
		self->sendVoiceMessage(VoiceMessage::AFFIRMATIVE);
	}
	item = nullptr;
	return true;
}

bool GiveAmmoAction::goalComplete() {
	if (!GoToAction::goalComplete()) {
		return false;
	}
	extern IServerPluginHelpers *helpers;
	helpers->ClientCommand(self->getEdict(), "dropammo");
	return true;
}

bool GiveAmmoAction::receive(edict_t *sender, const CCommand &command) {
	Player *player = Player::getPlayer(sender);
	if (player != nullptr && player->getTeam() == self->getTeam()
			&& self->getCurrentPosition().DistTo(
					player->getCurrentPosition()) < 500.0f
			&& self->isVoiceMessageType(DODVoiceMessage::NEED_AMMO, command.Arg(0))
			&& self->canSee(*player)) {
		item = sender;
		self->setResetPlanner(true);
		return true;
	}
	return false;
}
