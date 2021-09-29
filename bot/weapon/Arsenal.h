#pragma once

#include "WeaponBuilderFactory.h"
#include <goap/WorldCond.h>

class Weapon;
class Blackboard;
struct edict_t;

typedef bool (*WeaponFilter)(const Weapon*, Blackboard& blackboard, float dist);

class Arsenal {
public:
	static const char* getWeaponName(int key);

	Arsenal();

	~Arsenal() {
		reset();
	}

	int getBestWeapon(Blackboard& blackboard, const WeaponFilter& ignore) const;

	void reset();

	void update(edict_t* self);

	int getCurrWeaponIdx() const {
		return currWeapIdx;
	}

	/**
	 * @return The current Weapon
	 */
	Weapon *getCurrWeapon() const {
		return getWeapon(currWeapIdx);
	}

	Weapon* getWeapon(int key) const;

	CUtlMap<int, Weapon*>& getWeapons() {
		return weapons;
	}

	int getDesiredWeaponIdx() const {
		return desiredWeapIdx;
	}

	void setDesiredWeaponIdx(int desiredWeapon) {
		this->desiredWeapIdx = desiredWeapon;
	}

	int getBestWeaponIdx() const {
		return bestWeapIdx;
	}

	void setBestWeaponIdx(int bestWeapon) {
		this->bestWeapIdx = bestWeapon;
	}

	WeaponBuilderFactory& getWeaponFactory() {
		return factory;
	}

private:
	int currWeapIdx, bestWeapIdx, desiredWeapIdx = 0;

	CUtlMap<int, Weapon*> weapons;

	WeaponBuilderFactory factory;
};
