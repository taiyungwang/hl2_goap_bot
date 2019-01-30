#pragma once

#include "GoToAction.h"

class SnipeAction: public GoToAction {
public:
	SnipeAction(Blackboard& blackboard);

	virtual float getCost() const {
		return 5.0f;
	}

	bool execute();

	bool precondCheck();

private:
	bool deployed = false;

	float facing = 0;

	int duration = 0;
};
