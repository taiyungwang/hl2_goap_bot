#pragma once

#include <goap/action/WeaponAction.h>

class DeployMGAction: public WeaponAction {
public:
	DeployMGAction(Blackboard& blackboard);

	bool precondCheck();

	bool postCondCheck();

	bool execute();

	bool isInterruptable() const {
		return true;
	}

private:
	int position = 0,
	executionTime = 0;
};
