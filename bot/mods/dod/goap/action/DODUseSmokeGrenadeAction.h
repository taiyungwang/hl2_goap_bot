#pragma once

#include "DODThrowLiveGrenadeAction.h"

class Player;

class DODUseSmokeGrenadeAction: public DODThrowLiveGrenadeAction {
public:
	explicit DODUseSmokeGrenadeAction(Bot *self) :
		DODThrowLiveGrenadeAction(self) {
		effects = {WorldProp::ENEMY_SIGHTED, false};
	}

	bool precondCheck() override;

protected:
	bool canUse(const char* weaponName) const override;
};
