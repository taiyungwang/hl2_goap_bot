/*
 * Player.h
 *
 *  Created on: Feb 20, 2017
 */

#ifndef UTILS_MYBOT_PLAYER_H_
#define UTILS_MYBOT_PLAYER_H_

#include "GameEventListener.h"
#include <weapon/WeaponBuilder.h>
#include <functional>

class IPlayerInfo;
class Vector;
class Weapon;
class CNavArea;
class QAngle;
class WeaponBuilder;


class Player: public GameEventListener {
public:
	using TeamCount = std::tuple<int, int>;

	static std::unordered_map<int, Player*>& getPlayers() {
		return players;
	}

	static TeamCount getTeamCount();

	static Player* getPlayer(edict_t* ent);

	static Player* getPlayer(int idx) {
		return players.find(idx) != players.end() ?
				players[idx] : nullptr;
	}

	static Player *getPlayerByUserId(int userId);

	Player(edict_t* ent, const WeaponBuilders& arsenal);

	virtual ~Player();

	Vector getCurrentPosition() const;

	edict_t* getEdict() const {
		return ent;
	}

	const char* getName() const;

	int getTeam() const;

	bool isEnemy(const Player& other) const;

	const char* getWeaponName() const;

	virtual void think();

	int getUserId() const;

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

	virtual void FireGameEvent(IGameEvent* event);

	float getNoiseRange() const {
		return noiseRange;
	}

	bool isInGame() const {
		return inGame;
	}

	CNavArea *getArea() const;

	int getClosestHidingSpot() const;

	int getCurrWeaponIdx() const;

	std::shared_ptr<Weapon> getCurrWeapon();

	std::shared_ptr<Weapon> getWeapon(int id) const;

	int getWeaponId(const std::string& name) const;

	std::shared_ptr<Weapon> getWeapon(const std::string& name) const;

	void forMyWeapons(const std::function<bool(edict_t*)>& func) const;

protected:
	static std::unordered_map<int, Player*> players;

	bool inGame = true;

	int getBestWeapon(edict_t *target) const;
private:
	IPlayerInfo* info;

	edict_t* ent;

	const WeaponBuilders& weaponBuilders;

	int currWeapIdx = 0;

	/**
	 * Cache current weapon, since some weapons have state information.
	 */
	std::shared_ptr<Weapon> currentWeapon;

	float noiseRange = 0.0f;
};

#endif /* UTILS_MYBOT_PLAYER_H_ */
