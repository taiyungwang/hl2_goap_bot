#pragma once

#include <player/World.h>

class HL2DMWorld: public World {
private:
	void addStates() override;

	bool update(Blackboard &blackboard) override {
		return false;
	}
};
