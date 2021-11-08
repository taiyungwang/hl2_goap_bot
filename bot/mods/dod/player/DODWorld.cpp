#include "DODWorld.h"

#include <mods/dod/voice/DODVoiceMessage.h>
#include <event/EventInfo.h>
#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <voice/GrenadeVoiceMessage.h>
#include <voice/VoiceMessageSender.h>
#include <util/EntityUtils.h>

void DODWorld::addStates() {
	states.Insert(WorldProp::ALL_POINTS_CAPTURED, false);
	states.Insert(WorldProp::POINTS_DEFENDED, false);
	states.Insert(WorldProp::HAS_BOMB, false);
	states.Insert(WorldProp::BOMB_DEFUSED, false);
}

bool DODWorld::handle(EventInfo* event) {
	CUtlString name(event->getName());
	bool bombPlanted = name == "dod_bomb_planted";
	if (name == "dod_point_captured" || bombPlanted
			|| name == "dod_bomb_exploded" || name == "dod_bomb_defused") {
		for (auto player: Player::getPlayers()) {
			if (player.second->getUserId() == event->getInt("userid")) {
				bombPlantTeam = bombPlanted ? player.second->getTeam() : 1;
				break;
			}
		}
		reset = true;
		return false;
	}
	if (name == "dod_round_active") {
		if (!roundStarted) {
			reset = true;
		}
		roundStarted = true;
		bombPlantTeam = 1;
	} else if (name == "dod_round_win" || name == "dod_game_over") {
		roundStarted = false;
		reset = true;
	}
	updateState(WorldProp::ROUND_STARTED, roundStarted);
	return false;
}

bool DODWorld::update(Blackboard& blackboard) {
	if (reset) {
		reset = false;
		return true;
	}
	Bot *self = blackboard.getSelf();
	int team = self->getTeam();
	if (getState(WorldProp::OUT_OF_AMMO)) {
		self->getVoiceMessageSender().sendMessage(std::make_shared<DODVoiceMessage::NeedAmmo>(self->getEdict()));
	}
	for (auto i: blackboard.getSelf()->getVision().getVisibleEntities()) {
		extern IVEngineServer *engine;
		edict_t *entity = engine->PEntityOfEntIndex(i);
		if (entity != nullptr && !entity->IsFree()
				&& ((team == 2 && (FClassnameIs(entity, "grenade_frag_ger")
				|| FClassnameIs(entity, "grenade_riflegren_ger")))
				|| (team == 3 && (FClassnameIs(entity, "grenade_frag_us")
						|| FClassnameIs(entity, "grenade_riflegren_us"))))
				&& self->getCurrentPosition().DistTo(entity->GetCollideable()->GetCollisionOrigin()) < 100.0f) {
			updateState(WorldProp::EXPLOSIVE_NEAR, true);
			self->getVoiceMessageSender().sendMessage(std::make_shared<GrenadeVoiceMessage>(self->getEdict()));
			break;
		}
	}
	updateState(WorldProp::HAS_BOMB,
			blackboard.getSelf()->getArsenal().getWeaponIdByName("weapon_basebomb") != 0);
	return false;
}
