#include "DODDefuseBombAction.h"

#include <mods/dod/player/DODObjectives.h>
#include <mods/dod/player/DODObjective.h>
#include <mods/dod/util/DodPlayer.h>
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <voice/NeedBackupVoiceMessage.h>
#include <voice/VoiceMessageSender.h>
#include <nav_mesh/nav.h>
#include <util/UtilTrace.h>
#include <in_buttons.h>

DODDefuseBombAction::DODDefuseBombAction(Blackboard& blackboard) :
	CapturePointAction(blackboard) {
	effects = {WorldProp::BOMB_DEFUSED, true};
}

bool DODDefuseBombAction::findTargetLoc() {
	if (!CapturePointAction::findTargetLoc()) {
		return false;
	}
	targetRadius = 30.0f;
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
		interruptable = true;
		return true;
	}
	if (!isAvailable(item)) {
		return true;
	}
	interruptable = false;
	Bot *self = blackboard.getSelf();
	self->getVoiceMessageSender().sendMessage(std::make_shared<NeedBackupVoiceMessage>(self->getEdict()));
	blackboard.getButtons().hold(IN_USE);
	Vector itemPos = UTIL_FindGround(
				item->GetCollideable()->GetCollisionOrigin());
	itemPos.z += HumanEyeHeight - 15.0f;
	blackboard.setViewTarget(itemPos);
	return false;
}

bool DODDefuseBombAction::isAvailable(edict_t* ent) {
	if (!CapturePointAction::isAvailable(ent)) {
		return false;
	}
	for (auto player: Player::getPlayers()) {
		if (player.second != blackboard.getSelf()
				&& player.second->getTeam() == blackboard.getSelf()->getTeam()
				&& player.second->getCurrentPosition().DistTo(ent->GetCollideable()->GetCollisionOrigin()) < 100.0f
				&& isTeammateActing(player.second->getEdict())) {
			return false;
		}
	}
	return true;
}

bool DODDefuseBombAction::isAvailable(const DODObjective& obj) {
	return objectives->isDetonation()
			&& blackboard.getSelf()->getTeam() == obj.getOwner()
			&& obj.hasBombs()
			&& obj.hasBombTargetInState(DODObjective::BombState::ACTIVE);
}

bool DODDefuseBombAction::isTeammateActing(edict_t* teammate) const {
	return DodPlayer(teammate).isDefusing();
}
