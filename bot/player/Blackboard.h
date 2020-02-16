#pragma once

#include <weapon/Armory.h>
#include "Buttons.h"
#include <eiface.h>
#include <iplayerinfo.h>

class BasePlayer;
class Player;
class Bot;
class CNavArea;
class Navigator;
struct edict_t;

class Blackboard {
public:

	static float clamp180(float angle);

	Blackboard(const CUtlMap<int, Player*>& players, Bot* player,
			BasePlayer* entInstance);

	void reset();

	~Blackboard();

	Buttons& getButtons() {
		return buttons;
	}

	void setViewTarget(const Vector& target) {
		this->viewTarget = target;
	}

	const Vector& getViewTarget() const {
		return viewTarget;
	}

	bool isOnLadder();

	float getAimAccuracy(const Vector& pos) const;

	float getTargetEntDistance() const;

	const Bot* getSelf() const {
		return self;
	}

	const Player* getTargetedPlayer() const {
		return targetedPlayer;
	}

	void setTargetedPlayer(const Player* targetedPlayer) {
		this->targetedPlayer = targetedPlayer;
	}

	edict_t* getBlocker() const {
		return blocker;
	}

	void setBlocker(edict_t* blocker) {
		this->blocker = blocker;
	}

	const CUtlMap<int, Player*>& getPlayers() const {
		return players;
	}

	CUtlVector<int>& getVisibleEnemies() {
		return visibleEnemies;
	}

	CBotCmd& getCmd() {
		return cmd;
	}

	Armory& getArmory() {
		return armory;
	}

	Navigator* getNavigator() {
		return navigator;
	}

	void setNavigator(Navigator* navigator) {
		this->navigator = navigator;
	}

	void lookStraight();

	void setStartArea(CNavArea* startArea) {
		this->startArea = startArea;
	}

	CNavArea* getStartArea() const {
		return startArea;
	}

	edict_t* getTarget() const {
		return target;
	}

	void setTarget(edict_t* target) {
		this->target = target;
	}
private:
	const Bot* self;

	const CUtlMap<int, Player*>& players;

	Armory armory;

	const Player *targetedPlayer;

	CUtlVector<int> visibleEnemies;

	CBotCmd cmd;

	Navigator* navigator;

	edict_t* blocker = nullptr,
		* target = nullptr;

	Vector viewTarget;

	Buttons buttons;

	BasePlayer* entInstance;

	// Used to track start areas for chained GoToActions.
	CNavArea* startArea = nullptr;
};
