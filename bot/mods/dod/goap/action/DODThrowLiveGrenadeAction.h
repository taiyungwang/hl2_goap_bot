#pragma once

#include <goap/action/ThrowGrenadeAction.h>

class DODThrowLiveGrenadeAction: public ThrowGrenadeAction {
public:
	explicit DODThrowLiveGrenadeAction(Bot *self) :
		ThrowGrenadeAction(self) {
		effects = { WorldProp::HAS_LIVE_GRENADE, false };
	}

private:
	const Player *chooseTarget() const override;

	bool canUse(const char *weaponName) const override;
};
