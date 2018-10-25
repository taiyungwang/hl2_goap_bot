#pragma once

#include "WeaponBuilderFactory.h"
#include <goap/WorldCond.h>

class Weapon;
class Blackboard;

typedef bool (*WeaponFilter)(const Weapon*, Blackboard& blackboard, float dist);

class Armory {
public:
	Armory();

	int getBestWeapon(Blackboard& blackboard, const WeaponFilter& ignore) const;

	void reset();

	void update(Blackboard& blackboard);

	int getCurrWeaponIdx() const {
		return currWeapIdx;
	}

	/**
	 * @return The current Weapon
	 */
	Weapon *getCurrWeapon() const {
		return getWeapon(currWeapIdx);
	}

	const char* getCurrentWeaponName() const {
		return getWeaponName(currWeapIdx);
	}

	const char* getWeaponName(int key) const;

	Weapon* getWeapon(int key) const;

	void setCurrentWeaponIdx(int currentWeapon) {
		this->currWeapIdx = currentWeapon;
	}

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
