#pragma once

#include <player/World.h>
#include <event/EventHandler.h>

class DODWorld: public World, public EventHandler {
public:
	DODWorld(bool roundStarted) :
			World(roundStarted), EventHandler() {
	}

	bool handle(EventInfo* event);

private:
	bool reset = false;

	void addStates();

	bool update(Blackboard& blackboard);
};
