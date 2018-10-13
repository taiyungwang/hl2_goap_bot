#pragma once

#include "WeaponAction.h"

/**
 * Switches to the best weapon ignoring clip status.
 */
class SwitchWeaponAction: public WeaponAction {
public:
	SwitchWeaponAction(Blackboard& blackboard);

	bool precondCheck();

	bool execute();
};
