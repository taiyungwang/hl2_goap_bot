#include "DODWorld.h"

#include <mods/dod/voice/DODVoiceMessage.h>
#include <mods/dod/weapon/DODLiveGrenadeBuilder.h>
#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <nav_mesh/nav.h>
#include <util/EntityUtils.h>
#include <util/BaseGrenade.h>

extern IGameEventManager2* gameeventmanager;

DODWorld::DODWorld() {
	gameeventmanager->AddListener(this, "dod_bomb_planted", true);
	gameeventmanager->AddListener(this, "dod_point_captured", true);
	gameeventmanager->AddListener(this, "dod_bomb_exploded", true);
	gameeventmanager->AddListener(this, "dod_bomb_defused", true);
	gameeventmanager->AddListener(this, "dod_round_active", true);
	gameeventmanager->AddListener(this, "dod_round_win", true);
	gameeventmanager->AddListener(this, "dod_game_over", true);
}

DODWorld::~DODWorld() {
	gameeventmanager->RemoveListener(this);
}

void DODWorld::addStates() {
	states[WorldProp::ALL_POINTS_CAPTURED] = false;
	states[WorldProp::POINTS_DEFENDED] = false;
	states[WorldProp::HAS_BOMB] = false;
	states[WorldProp::BOMB_DEFUSED] = false;
	states[WorldProp::HAS_LIVE_GRENADE] = false;
	states[WorldProp::HEARD_NEED_AMMO] = true;
}

void DODWorld::FireGameEvent(IGameEvent* event) {
	std::string name(event->GetName());
	bool bombPlanted = name == "dod_bomb_planted";
	if (name == "dod_point_captured" || bombPlanted
			|| name == "dod_bomb_exploded" || name == "dod_bomb_defused") {
		reset = true;
		return;
	}
	if (name == "dod_round_active") {
		if (!roundStarted) {
			reset = true;
		}
		roundStarted = true;
	} else if (name == "dod_round_win" || name == "dod_game_over") {
		roundStarted = false;
		reset = true;
	}
	updateState(WorldProp::ROUND_STARTED, roundStarted);
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
				self->sendVoiceMessage(DODVoiceMessage::GRENADE);
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
			self->sendVoiceMessage(DODVoiceMessage::MG_AHEAD);
			break;
		}
		if (weap == "weapon_spring" || weap == "weapon_k98_scoped") {
			self->sendVoiceMessage(DODVoiceMessage::SNIPER);
			break;
		}
		if (weap == "weapon_bazooka" || weap == "weapon_pschreck") {
			self->sendVoiceMessage(DODVoiceMessage::ROCKET_AHEAD);
			break;
		}
	}
	if (getState(WorldProp::OUT_OF_AMMO) && baseBombId != weapIdx
			&& !arsenal.getWeapon(weapIdx)->isGrenade()) {
		self->sendVoiceMessage(DODVoiceMessage::NEED_AMMO);
	}
	return false;
}
