#pragma once

#include <player/World.h>

class HL2DMWorld: public World {
public:
	HL2DMWorld() :
			World(true) {
	}

private:
	void addStates();

	bool update(Blackboard& blackboard);
};
