#pragma once

#include "Buttons.h"
#include <eiface.h>
#include <iplayerinfo.h>

class BasePlayer;
class Player;
class Bot;
class Navigator;
class CGameTrace;
struct edict_t;

class Blackboard {
public:

	static float clamp180(float angle);

	Blackboard(Bot* player,
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

	Bot* getSelf() {
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

	CUtlVector<int>& getVisibleEnemies() {
		return visibleEnemies;
	}

	CBotCmd& getCmd() {
		return cmd;
	}

	Navigator* getNavigator() {
		return navigator;
	}

	void setNavigator(Navigator* navigator) {
		this->navigator = navigator;
	}

	void lookStraight();

	edict_t* getTarget() const {
		return target;
	}

	void setTarget(edict_t* target) {
		this->target = target;
	}

	Vector getFacing() const;

	bool checkVisible(const Vector &vecAbsEnd, edict_t* target) const;

	bool checkVisible(CGameTrace& result, const Vector &vecAbsEnd, edict_t* target) const;

private:
	Bot* self;

	const Player *targetedPlayer;

	CUtlVector<int> visibleEnemies;

	CBotCmd cmd;

	Navigator* navigator;

	edict_t* blocker = nullptr,
		* target = nullptr;

	Vector viewTarget;

	Buttons buttons;

	BasePlayer* entInstance;
};
