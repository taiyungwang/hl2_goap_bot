/*
 * Player.h
 *
 *  Created on: Feb 20, 2017
 */

#ifndef UTILS_MYBOT_PLAYER_H_
#define UTILS_MYBOT_PLAYER_H_

#include <igameevents.h>
#include <unordered_map>
#include <memory>

class IPlayerInfo;
class Vector;
class Arsenal;
class CNavArea;
class QAngle;
struct edict_t;

class Player: public IGameEventListener2 {
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

	Player(edict_t* ent, const std::shared_ptr<Arsenal>& arsenal);

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

	Arsenal& getArsenal() {
		return *arsenal.get();
	}

	const Arsenal& getArsenal() const {
		return *arsenal.get();
	}

	virtual void FireGameEvent(IGameEvent* event);

	float getNoiseRange() const {
		return noiseRange;
	}

	bool isInGame() const {
		return inGame;
	}

	CNavArea *getArea() const {
		return area;
	}

	int getClosestHidingSpot() const;

protected:
	static std::unordered_map<int, Player*> players;

	std::shared_ptr<Arsenal> arsenal;

	bool inGame = true;

	CNavArea *area = nullptr;

private:
	IPlayerInfo* info;

	edict_t* ent;

	float noiseRange = 0.0f;
};

#endif /* UTILS_MYBOT_PLAYER_H_ */
