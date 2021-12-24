#pragma once

#include "SwitchToDesiredWeaponAction.h"

/**
 * Switches to the best weapon ignoring clip status.
 */
class SwitchWeaponAction: public SwitchToDesiredWeaponAction {
public:
	SwitchWeaponAction(Blackboard& blackboard);

	bool precondCheck();
};
