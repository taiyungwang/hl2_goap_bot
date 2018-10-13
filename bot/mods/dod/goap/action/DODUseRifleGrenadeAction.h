#pragma once

#include "DODUseSmokeGrenadeAction.h"

class Player;

class DODUseRifleGrenadeAction: public DODUseSmokeGrenadeAction {
public:
	DODUseRifleGrenadeAction(Blackboard& blackboard) :
			DODUseSmokeGrenadeAction(blackboard) {
		effects = {WorldProp::MULTIPLE_ENEMY_SIGHTED, false};
	}

	virtual bool precondCheck();

protected:

	const Player* target = nullptr;

	virtual bool canUse(const char* weaponName) const;
};
