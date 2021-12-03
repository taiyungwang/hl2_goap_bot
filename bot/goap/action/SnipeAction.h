#pragma once

#include "GoToAction.h"

class HidingSpotSelector;

class SnipeAction: public GoToAction {
public:
	static void setSpotSelector(HidingSpotSelector* sel) {
		selector = sel;
	}

	SnipeAction(Blackboard& blackboard);

	bool onPlanningFinished();

	bool execute();

	bool goalComplete();

	void abort();

protected:
	static HidingSpotSelector* selector;

	int duration = 0;

	int selectorId = -1;

	QAngle facing;

	float calculateFacing();

	virtual bool findTargetLoc();
};
