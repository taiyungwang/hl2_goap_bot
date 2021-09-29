/*
 * Player.h
 *
 *  Created on: Feb 20, 2017
 */

#ifndef UTILS_MYBOT_PLAYER_H_
#define UTILS_MYBOT_PLAYER_H_

#include <event/EventHandler.h>
#include <weapon/Arsenal.h>
#include <utlmap.h>

class IPlayerInfo;
class Vector;
class QAngle;
struct edict_t;

class Player: public EventHandler {
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

	Vector getCurrentPosition() const;

	edict_t* getEdict() const {
		return ent;
	}

	const char* getName() const;

	int getTeam() const;

	const char* getWeaponName() const;

	virtual void think();

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

	IPlayerInfo* getInfo() const {
		return info;
	}

	Arsenal& getArsenal() {
		return arsenal;
	}

	virtual bool handle(EventInfo* event);

	float getNoiseRange() const {
		return noiseRange;
	}

protected:
	static CUtlMap<int, Player*> players;

	Arsenal arsenal;

	bool inGame = false;

private:
	IPlayerInfo* info;

	edict_t* ent;

	float noiseRange = 0.0f;
};

#endif /* UTILS_MYBOT_PLAYER_H_ */
