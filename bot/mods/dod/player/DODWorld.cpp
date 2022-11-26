#include "DODWorld.h"

#include <mods/dod/voice/DODVoiceMessage.h>
#include <mods/dod/weapon/DODLiveGrenadeBuilder.h>
#include <event/EventInfo.h>
#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <voice/GrenadeVoiceMessage.h>
#include <voice/VoiceMessageSender.h>
#include <nav_mesh/nav.h>
#include <util/EntityUtils.h>
#include <util/BaseGrenade.h>

void DODWorld::addStates() {
	states[WorldProp::ALL_POINTS_CAPTURED] = false;
	states[WorldProp::POINTS_DEFENDED] = false;
	states[WorldProp::HAS_BOMB] = false;
	states[WorldProp::BOMB_DEFUSED] = false;
	states[WorldProp::HAS_LIVE_GRENADE] = false;
	states[WorldProp::HEARD_NEED_AMMO] = true;
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
	auto arsenal = blackboard.getSelf()->getArsenal();
	int baseBombId = arsenal.getWeaponIdByName("weapon_basebomb"),
			weapIdx = arsenal.getCurrWeaponIdx();
	if (getState(WorldProp::OUT_OF_AMMO) && baseBombId != weapIdx
			&& !arsenal.getWeapon(weapIdx)->isGrenade()) {
		self->getVoiceMessageSender().sendMessage(std::make_shared<DODVoiceMessage::NeedAmmo>(self->getEdict()));
	}
	updateState(WorldProp::HAS_BOMB, baseBombId != 0);
	updateState(WorldProp::HAS_LIVE_GRENADE, false);
	for (auto name: DODLiveGrenadeBuilder::NAMES) {
		if (arsenal.getWeaponIdByName(name.c_str()) > 0) {
			updateState(WorldProp::HAS_LIVE_GRENADE, true);
			return true;
		}
	}
	for (auto i: blackboard.getSelf()->getVision().getVisibleEntities()) {
		extern IVEngineServer *engine;
		edict_t *entity = engine->PEntityOfEntIndex(i);
		if (entity == nullptr || entity->IsFree()) {
			continue;
		}
		BaseGrenade grenade(entity);
		if ((grenade.getThrower() == self->getEdict()
				|| ((team == 2 && (FClassnameIs(entity, "grenade_frag_ger")
						|| FClassnameIs(entity, "grenade_riflegren_ger")))
						|| (team == 3 && (FClassnameIs(entity, "grenade_frag_us")
								|| FClassnameIs(entity, "grenade_riflegren_us")))))
				&& self->getCurrentPosition().DistTo(entity->GetCollideable()->GetCollisionOrigin())
				< *grenade.getDmgRadius() + HalfHumanWidth) {
			updateState(WorldProp::EXPLOSIVE_NEAR, true);
			if (grenade.getThrower() != self->getEdict()) {
				self->getVoiceMessageSender().sendMessage(std::make_shared<GrenadeVoiceMessage>(self->getEdict()));
			}
			return true;
		}
	}
	for (int i : self->getVision().getVisibleEnemies()) {
		const Player *enemy = Player::getPlayer(i);
		if (enemy == nullptr) {
			continue;
		}
		std::string weap(enemy->getWeaponName());
		if (weap == "weapon_30cal" || weap == "weapon_mg42") {
			self->getVoiceMessageSender().sendMessage(std::make_shared<DODVoiceMessage::MGAheadVoiceMessage>(self->getEdict()));
			break;
		}
		if (weap == "weapon_spring" || weap == "weapon_k98_scoped") {
			self->getVoiceMessageSender().sendMessage(std::make_shared<DODVoiceMessage::SniperAheadVoiceMessage>(self->getEdict()));
			break;
		}
		if (weap == "weapon_bazooka" || weap == "weapon_pschreck") {
			self->getVoiceMessageSender().sendMessage(std::make_shared<DODVoiceMessage::RocketAheadVoiceMessage>(self->getEdict()));
			break;
		}
	}
	return false;
}
