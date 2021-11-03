#pragma once

#include "Buttons.h"
#include <eiface.h>
#include <iplayerinfo.h>

class BasePlayer;
class Bot;
class Navigator;
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

	Bot* getSelf() {
		return self;
	}

	edict_t* getBlocker() const {
		return blocker;
	}

	void setBlocker(edict_t* blocker) {
		this->blocker = blocker;
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

private:
	Bot* self;

	CBotCmd cmd;

	Navigator* navigator;

	edict_t* blocker = nullptr,
		* target = nullptr;

	Vector viewTarget;

	Buttons buttons;

	BasePlayer* entInstance;
};
