#include "Player.h"

#include <eiface.h>
#include <iplayerinfo.h>

CUtlMap<int, Player*> Player::players;

extern IVEngineServer* engine;

Player* Player::getPlayer(edict_t* ent) {
	return getPlayer(engine->IndexOfEdict(ent));
}

Player::Player(edict_t* ent) :
		ent(ent) {
	extern IPlayerInfoManager *playerinfomanager;
	info = playerinfomanager->GetPlayerInfo(ent);
	players.Insert(engine->IndexOfEdict(ent), this);
}

Player::~Player() {
	// Cant' rely on server index for server shutting down.
	FOR_EACH_MAP_FAST(players, i) {
		if (players[i] == this) {
			players.RemoveAt(i);
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

bool Player::isDead() const {
	return ent->IsFree() || info->IsDead() || info->GetHealth() <= 0;
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

QAngle Player::getFacingAngle() const {
	return info->GetLastUserCommand().viewangles;
}

QAngle Player::getAbsoluteAngle() const {
	return info->GetAbsAngles();
}
