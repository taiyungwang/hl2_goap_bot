#pragma once

#include <player/World.h>
#include <event/EventHandler.h>

class DODWorld: public World, public EventHandler {
public:
	DODWorld(bool roundStarted) :
			World(roundStarted) {
	}

	bool handle(EventInfo* event);

private:
	bool reset = false;

	int bombPlantTeam = 1;

	void addStates();

	bool update(Blackboard& blackboard);
};
