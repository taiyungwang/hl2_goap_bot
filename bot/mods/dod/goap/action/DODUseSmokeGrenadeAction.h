#pragma once

#include "DODThrowLiveGrenadeAction.h"

class Player;

class DODUseSmokeGrenadeAction: public DODThrowLiveGrenadeAction {
public:
	DODUseSmokeGrenadeAction(Blackboard& blackboard) :
		DODThrowLiveGrenadeAction(blackboard) {
		effects = {WorldProp::MULTIPLE_ENEMY_SIGHTED, false};
	}

	bool precondCheck() override;

protected:
	virtual bool canUse(const char* weaponName) const override;
};
