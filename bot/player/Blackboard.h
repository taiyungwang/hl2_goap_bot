#pragma once

#include "Buttons.h"
#include <eiface.h>
#include <iplayerinfo.h>

class BasePlayer;
class Bot;
class Navigator;

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

	bool isOnLadder();

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

	edict_t* getTarget() const {
		return target;
	}

	void setTarget(edict_t* target) {
		this->target = target;
	}

private:
	Bot* self;

	CBotCmd cmd;

	edict_t* blocker = nullptr,
		* target = nullptr;

	Buttons buttons;

	BasePlayer* entInstance;
};
