#pragma once

#include "Action.h"

class ReloadWeaponAction: public Action {
public:
	ReloadWeaponAction(Bot *self);

	bool precondCheck();

	bool execute();

	bool isInterruptable() const {
		return false;
	}
};
