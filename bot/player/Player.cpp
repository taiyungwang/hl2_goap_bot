#include "Player.h"

#include <eiface.h>
#include <iplayerinfo.h>

Player::Player(edict_t* ent) :
		ent(ent) {
	extern IPlayerInfoManager *playerinfomanager;
	info = playerinfomanager->GetPlayerInfo(ent);
	extern IVEngineServer* engine;
	userId = engine->GetPlayerUserId(ent);
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
	return info->IsDead() || info->GetHealth() <= 0;
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

QAngle Player::getNormalizedAngle() const {
	return info->GetLastUserCommand().viewangles;
}

Vector Player::getFacing() const {
	Vector facing;
	AngleVectors(info->GetAbsAngles(), &facing);
	return facing;
}
