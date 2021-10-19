#include "Player.h"

#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>
#include <weapon/WeaponFunction.h>
#include <event/EventInfo.h>
#include <util/BasePlayer.h>
#include <eiface.h>
#include <iplayerinfo.h>
#include <in_buttons.h>

std::unordered_map<int, Player*> Player::players;

extern IVEngineServer* engine;

Player* Player::getPlayer(edict_t* ent) {
	return getPlayer(engine->IndexOfEdict(ent));
}

Player::Player(edict_t* ent, const std::shared_ptr<Arsenal>& arsenal) :
		ent(ent), arsenal(arsenal) {
	extern IPlayerInfoManager *playerinfomanager;
	info = playerinfomanager->GetPlayerInfo(ent);
	players[engine->IndexOfEdict(ent)] = this;
}

Player::~Player() {
	// Cant' rely on server index for server shutting down.
	for(auto player: players) {
		if (player.second == this) {
			players.erase(player.first);
			return;
		}
	}
}

const char* Player::getName() const {
	return info->GetName();
}

int Player::getTeam() const {
	return info->GetTeamIndex();
}

Vector Player::getCurrentPosition() const {
	return ent->GetCollideable()->GetCollisionOrigin();
}

const char* Player::getWeaponName() const {
	return info->GetWeaponName();
}

void Player::think() {
	if (inGame && (ent->IsFree() || info->IsDead() || info->GetHealth() <= 0)) {
		inGame = false;
	}
	if (inGame) {
		noiseRange = 0.0f;
		arsenal->update(ent);
		if (BasePlayer(ent).getVelocity().Length() > 150.0f) {
			noiseRange = 100.0f;
		}
		int buttons = info->GetLastUserCommand().buttons;
		Weapon *currWeap = arsenal->getCurrWeapon();
		if (currWeap != nullptr
				&& (((buttons & IN_ATTACK) && !currWeap->getPrimary()->isSilent())
						|| ((buttons & IN_ATTACK2) && currWeap->getSecondary() != nullptr
								&& !currWeap->getSecondary()->isSilent()))) {
			noiseRange = 1000.0f;
		}

	}
}


int Player::getUserId() const {
	extern IVEngineServer* engine;
	return engine->GetPlayerUserId(ent);
}

Vector Player::getEyesPos() const {
	Vector pos;
	extern IServerGameClients* gameclients;
	gameclients->ClientEarPosition(ent, &pos);
	return pos;
}

int Player::getArmor() const {
	return info->GetArmorValue();
}

int Player::getHealth() const {
	return info->GetHealth();
}

int Player::getMaxHealth() const {
	return info->GetMaxHealth();
}

bool Player::handle(EventInfo* event) {
	CUtlString name(event->getName());
	int eventUserId = event->getInt("userid");
	// bot owns this event.
	if (eventUserId == getUserId()) {
		if (name == "player_spawn") {
			arsenal->reset();
			inGame = true;
		}
		if (name == "player_death" && inGame) {
			inGame = false;
		}
	}
	return false;
}

QAngle Player::getFacingAngle() const {
	return info->GetLastUserCommand().viewangles;
}

QAngle Player::getAbsoluteAngle() const {
	return info->GetAbsAngles();
}
