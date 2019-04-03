#pragma once

#include "GoToAction.h"

class HidingSpotSelector;

class SnipeAction: public GoToAction {
public:
	static void setSpotSelector(HidingSpotSelector* sel) {
		selector = sel;
	}

	SnipeAction(Blackboard& blackboard);

	bool execute();

	bool precondCheck();

	bool postCondCheck();

	void abort();

private:

	static HidingSpotSelector* selector;

	bool deployed = false;

	float facing = 0;

	int duration = 0,
		selectorId = -1;

	bool findTargetLoc();
};
