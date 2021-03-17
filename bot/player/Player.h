/*
 * Player.h
 *
 *  Created on: Feb 20, 2017
 */

#ifndef UTILS_MYBOT_PLAYER_H_
#define UTILS_MYBOT_PLAYER_H_

#include <utlmap.h>

class IPlayerInfo;
class Vector;
class QAngle;
struct edict_t;

class Player {
public:
	static CUtlMap<int, Player*>& getPlayers() {
		return players;
	}

	static Player* build(edict_t* ent);

	static Player* getPlayer(edict_t* ent);

	static Player* getPlayer(int idx) {
		return players[players.Find(idx)];
	}

	Player(edict_t* ent);

	virtual ~Player();

	virtual Vector getCurrentPosition() const;

	edict_t* getEdict() const {
		return ent;
	}

	const char* getName() const;

	int getTeam() const;

	const char* getWeaponName() const;

	virtual void think() {
	}

	int getUserId() const;

	bool isDead() const;

	Vector getEyesPos() const;

	int getArmor() const;

	int getHealth() const;

	int getMaxHealth() const;

	/**
	 * @Return Angle given by the player's last issued command.
	 */
	QAngle getFacingAngle() const;

	QAngle getAbsoluteAngle() const;

protected:
	static CUtlMap<int, Player*> players;

private:
	IPlayerInfo* info;

	edict_t* ent;
};

#endif /* UTILS_MYBOT_PLAYER_H_ */
