#pragma once

#include "SwitchToDesiredWeaponAction.h"

/**
 * Switches to the next best loaded weapon.
 */
class SwitchToBestLoadedWeaponAction: public SwitchToDesiredWeaponAction {
public:
	SwitchToBestLoadedWeaponAction(Bot *self);

	bool precondCheck();

	float getCost() {
		return 1.5f;
	}
};
