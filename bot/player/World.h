#pragma once

#include <goap/WorldCond.h>

class Bot;

class World {
public:
	virtual ~World() {
	}

	void reset();

	bool getState(WorldProp prop) const {
		return states.at(prop);
	}

	bool think(Bot *self);

	void updateState(WorldProp name, bool cond) {
		states[name] = cond;
	}

	WorldState& getStates() {
		return states;
	}

protected:
	WorldState states;

	virtual void addStates() = 0;

	virtual bool update(Bot *self) = 0;
};
