/*
 * Player.h
 *
 *  Created on: Feb 20, 2017
 */

#ifndef UTILS_MYBOT_PLAYER_H_
#define UTILS_MYBOT_PLAYER_H_

#include <Thinker.h>

class IPlayerInfo;
class Vector;
class QAngle;
struct edict_t;

class Player: public Thinker {
public:

	Player(edict_t* ent);

	virtual ~Player() {
	}

	virtual Vector getCurrentPosition() const;

	edict_t* getEdict() const {
		return ent;
	}

	const char* getName() const;

	int getTeam() const;

	virtual void think() {
	}

	int getUserId() const {
		return userId;
	}

	bool isDead() const;

	Vector getEyesPos() const;

	int getArmor() const;

	QAngle getAngle() const;

	Vector getFacing() const;


private:

	IPlayerInfo* info;

	edict_t* ent;

	int userId;

};

#endif /* UTILS_MYBOT_PLAYER_H_ */
