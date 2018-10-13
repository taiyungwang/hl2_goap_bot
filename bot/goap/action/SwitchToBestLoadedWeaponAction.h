#pragma once

#include "WeaponAction.h"

/**
 * Switches to the next best loaded weapon.
 */
class SwitchToBestLoadedWeaponAction: public WeaponAction {
public:
	SwitchToBestLoadedWeaponAction(Blackboard& blackboard);

	bool precondCheck();

	bool execute();

private:
	int nextBest = -1;
};
