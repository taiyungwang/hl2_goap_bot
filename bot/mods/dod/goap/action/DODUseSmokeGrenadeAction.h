#pragma once

#include "DODThrowLiveGrenadeAction.h"

class Player;

class DODUseSmokeGrenadeAction: public DODThrowLiveGrenadeAction {
public:
	DODUseSmokeGrenadeAction(Blackboard& blackboard) :
		DODThrowLiveGrenadeAction(blackboard) {
		effects = {WorldProp::ENEMY_SIGHTED, false};
	}

	virtual bool precondCheck() override;

protected:
	virtual bool canUse(const char* weaponName) const override;
};
