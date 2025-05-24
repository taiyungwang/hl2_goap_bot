#pragma once

#include "Action.h"

class SwitchToDesiredWeaponAction: public Action {
public:
	SwitchToDesiredWeaponAction(Bot *self) :
			Action(self) {
		effects = {WorldProp::USING_DESIRED_WEAPON, true};
	}

	bool precondCheck() override;
	
	bool execute() override;

protected:
	int weapIdx = 0;
};
