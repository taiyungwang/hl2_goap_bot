#include "Player.h"

#include <util/BaseEntity.h>
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
	return BaseEntity(ent).getTeam();
}

Vector Player::getCurrentPosition() const {
	return ent->GetCollideable()->GetCollisionOrigin();
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

QAngle Player::getAngle() const {
	return info->GetLastUserCommand().viewangles;
}

Vector Player::getFacing() const {
	Vector facing;
	AngleVectors(getAngle(), &facing);
	return facing;
}
