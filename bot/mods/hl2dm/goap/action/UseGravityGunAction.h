#pragma once

#include <goap/action/UseSpecificWeaponAction.h>

class UseGravityGunAction: public UseSpecificWeaponAction {
public:
	UseGravityGunAction(Bot *self): UseSpecificWeaponAction(self) {
		effects = {WorldProp::IS_BLOCKED, false};
	}

	bool precondCheck();

	bool execute();

protected:
	virtual bool canUse(int weaponIdx) const;
};
