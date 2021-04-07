#pragma once

#include "WeaponAction.h"

/**
 * Switches to the next best loaded weapon.
 */
class SwitchToBestLoadedWeaponAction: public WeaponAction {
public:
	SwitchToBestLoadedWeaponAction(Blackboard& blackboard);

	bool precondCheck();

	float getCost() {
		return 1.5f;
	}

	bool execute();

private:
	int nextBest = -1;
};
