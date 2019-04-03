#pragma once

#include <goap/action/UseSpecificWeaponAction.h>

class UseGravityGunAction: public UseSpecificWeaponAction {
public:
	UseGravityGunAction(Blackboard& blackboard): UseSpecificWeaponAction(blackboard) {
		effects = {WorldProp::IS_BLOCKED, false};
	}

	bool precondCheck();

	bool execute();

protected:
	virtual bool canUse(const char* weaponName) const;
};
