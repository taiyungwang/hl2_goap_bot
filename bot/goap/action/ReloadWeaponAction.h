#pragma once

#include "Action.h"

class ReloadWeaponAction: public Action {
public:
	ReloadWeaponAction(Blackboard& blackboard);

	bool precondCheck();

	bool execute();

	bool isInterruptable() const {
		return false;
	}
};
