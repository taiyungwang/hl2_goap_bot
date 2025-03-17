#pragma once

#include "SwitchToDesiredWeaponAction.h"

/**
 * Switches to the best weapon ignoring clip status.
 */
class SwitchWeaponAction: public SwitchToDesiredWeaponAction {
public:
	SwitchWeaponAction(Bot *self);

	bool precondCheck();
};
