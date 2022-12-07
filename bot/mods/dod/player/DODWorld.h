#pragma once

#include <player/World.h>
#include <igameevents.h>

class DODWorld: public World, public IGameEventListener2 {
public:
	DODWorld(bool roundStarted);

	~DODWorld();

	void FireGameEvent(IGameEvent* event);

private:
	bool reset = false;

	int bombPlantTeam = 1;

	void addStates();

	bool update(Blackboard& blackboard);
};
