#pragma once

#include "WeaponAction.h"

class SwitchToDesiredWeaponAction: public WeaponAction {
public:
	SwitchToDesiredWeaponAction(Blackboard& blackboard) :
			WeaponAction(blackboard) {
		effects = {WorldProp::USING_DESIRED_WEAPON, true};
	}

	virtual bool precondCheck();

	void init();

	bool execute();
};
