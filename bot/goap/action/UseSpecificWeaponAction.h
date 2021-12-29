#pragma once

#include "WeaponAction.h"

class UseSpecificWeaponAction: public WeaponAction {
public:
	UseSpecificWeaponAction(Blackboard& blackboard);

	virtual bool precondCheck();

protected:
	int weapIdx = 0;

	virtual bool canUse(int weapIdx) const;
};
