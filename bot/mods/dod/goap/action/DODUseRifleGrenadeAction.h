#pragma once

#include "DODUseSmokeGrenadeAction.h"

class Player;

class DODUseRifleGrenadeAction: public DODUseSmokeGrenadeAction {
public:
	DODUseRifleGrenadeAction(Blackboard& blackboard) :
			DODUseSmokeGrenadeAction(blackboard) {
	}

	virtual bool precondCheck();

	virtual bool execute() override;


protected:
	const Player* target = nullptr;

	virtual bool canUse(const char* weaponName) const;
};
