#pragma once

#include "FindPathAction.h"

class SnipeAction: public FindPathAction {
public:
	SnipeAction(Blackboard& blackboard);

	virtual float getCost() const {
		return 5.0f;
	}

	bool execute();

	bool isInterruptable() const {
		return true;
	}

	bool precondCheck();

private:

	float facing = 0;

	int duration = 0;
};
