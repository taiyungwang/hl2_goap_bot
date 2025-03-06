#pragma once

#include "Action.h"

class SwitchToDesiredWeaponAction: public Action {
public:
	SwitchToDesiredWeaponAction(Blackboard& blackboard) :
			Action(blackboard) {
		effects = {WorldProp::USING_DESIRED_WEAPON, true};
	}

	virtual bool precondCheck();

	bool execute();
};
