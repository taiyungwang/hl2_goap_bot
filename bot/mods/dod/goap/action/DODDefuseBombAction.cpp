#include "DODDefuseBombAction.h"

#include <mods/dod/player/DODObjectives.h>
#include <mods/dod/player/DODObjective.h>
#include <mods/dod/util/DodPlayer.h>
#include <player/Bot.h>
#include <voice/VoiceMessage.h>
#include <nav_mesh/nav.h>
#include <util/UtilTrace.h>
#include <in_buttons.h>

DODDefuseBombAction::DODDefuseBombAction(Bot *self) :
	CapturePointAction(self) {
	effects = {WorldProp::BOMB_DEFUSED, true};
}

bool DODDefuseBombAction::precondCheck() {
	if (!CapturePointAction::precondCheck()) {
		return false;
	}
	targetRadius = 30.0f;
	interruptable = true;
	return true;
}

bool DODDefuseBombAction::execute() {
	if (isDepleted()) {
		return true;
	}
	if (!GoToEntityAction::execute()) {
		return false;
	}
	if (!GoToAction::goalComplete()) {
		return true;
	}
	interruptable = false;
	self->sendVoiceMessage(VoiceMessage::NEED_BACKUP);
	useItem(isActingOnBomb(self->getEdict()));
	return false;
}

bool DODDefuseBombAction::isAvailable(edict_t* ent) {
	if (!CapturePointAction::isAvailable(ent)) {
		return false;
	}
	for (auto player: Player::getPlayers()) {
		if (player.second != self
				&& player.second->getTeam() == self->getTeam()
				&& player.second->getCurrentPosition().DistTo(ent->GetCollideable()->GetCollisionOrigin()) < 100.0f
				&& isActingOnBomb(player.second->getEdict())) {
			return false;
		}
	}
	return true;
}

bool DODDefuseBombAction::isAvailable(const DODObjective& obj) {
	return objectives->isDetonation()
			&& self->getTeam() == obj.getOwner()
			&& obj.hasBombs()
			&& obj.hasBombTargetInState(DODObjective::BombState::ACTIVE);
}

bool DODDefuseBombAction::isActingOnBomb(edict_t* teammate) const {
	return DodPlayer(teammate).isDefusing();
}
