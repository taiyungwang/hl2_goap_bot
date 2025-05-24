#pragma once

#include "SwitchToDesiredWeaponAction.h"

/**
 * Switches to the best weapon ignoring clip status.
 */
class SwitchWeaponAction final : public SwitchToDesiredWeaponAction {
public:
	explicit SwitchWeaponAction(Bot *self) :
		SwitchToDesiredWeaponAction(self) {
		effects = {WorldProp::USING_BEST_WEAP, true};
	}

	bool precondCheck() override;
};
