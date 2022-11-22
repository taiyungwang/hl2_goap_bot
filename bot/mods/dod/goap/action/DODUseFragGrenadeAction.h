#pragma once

#include "DODUseRifleGrenadeAction.h"

class Player;

class DODUseFragGrenadeAction: public DODUseRifleGrenadeAction {
public:
	DODUseFragGrenadeAction(Blackboard& blackboard) :
			DODUseRifleGrenadeAction(blackboard) {
	}

	bool init() {
		primeDuration = 0;
		return true;
	}

	bool execute();

private:
	int primeDuration = 0;

	bool canUse(const char* weaponName) const;
};
