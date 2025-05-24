#pragma once

#include "SwitchToDesiredWeaponAction.h"

class UseSpecificWeaponAction: public SwitchToDesiredWeaponAction {
public:
	explicit UseSpecificWeaponAction(Bot *self): SwitchToDesiredWeaponAction(self) {
	}

	bool precondCheck() override;

	bool execute() final {
		return SwitchToDesiredWeaponAction::execute() && use();

	}

protected:
	virtual bool canUse(int weapIdx) const;

	virtual bool use() = 0;
};
