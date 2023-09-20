#pragma once

#include <player/World.h>
#include <player/GameEventListener.h>

class DODWorld: public World, public GameEventListener {
public:
	static void setRoundStarted(bool started) {
		roundStarted = started;
	}

	DODWorld();

	void FireGameEvent(IGameEvent* event);

private:
	static bool roundStarted;

	bool reset = false;

	int teamWithDeadDefuserOrPlanter = 0;

	void addStates();

	bool update(Blackboard& blackboard);
};
