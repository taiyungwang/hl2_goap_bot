#pragma once

#include <goap/WorldCond.h>

class Blackboard;

class World {
public:

	World(bool roundStarted = true): roundStarted(roundStarted) {
	}

	virtual ~World() {
	}

	void reset();

	bool getState(WorldProp prop) const {
		return states.at(prop);
	}

	bool think(Blackboard& blackboard);

	void updateState(WorldProp name, bool cond) {
		states[name] = cond;
	}

	WorldState& getStates() {
		return states;
	}

protected:
	bool roundStarted;

	WorldState states;

	virtual void addStates() = 0;

	virtual bool update(Blackboard& blackboard) = 0;
};
