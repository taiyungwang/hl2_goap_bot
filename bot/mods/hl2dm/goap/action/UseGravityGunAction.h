#pragma once

#include <goap/action/UseSpecificWeaponAction.h>

class UseGravityGunAction: public UseSpecificWeaponAction {
public:
	explicit UseGravityGunAction(Bot *self): UseSpecificWeaponAction(self) {
		effects = {WorldProp::IS_BLOCKED, false};
	}

	bool precondCheck() override;

private:
	bool use() override;

	bool canUse(int weaponIdx) const override;
};
