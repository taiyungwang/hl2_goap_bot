#pragma once

#include "SwitchToDesiredWeaponAction.h"

class UseSpecificWeaponAction: public SwitchToDesiredWeaponAction {
public:
	UseSpecificWeaponAction(Blackboard& blackboard);

	bool precondCheck() override;

protected:
	int weapIdx = 0;

	virtual bool canUse(int weapIdx) const;
};
