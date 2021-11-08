#pragma once

#include <goap/action/UseSpecificWeaponAction.h>

class Player;

class DODUseSmokeGrenadeAction: public UseSpecificWeaponAction {
public:
	DODUseSmokeGrenadeAction(Blackboard& blackboard) :
			UseSpecificWeaponAction(blackboard) {
		effects = {WorldProp::MULTIPLE_ENEMY_SIGHTED, false};
	}

	bool precondCheck();

	virtual bool execute();

protected:
	Vector viewTarget;

	const Player* target = nullptr;

	virtual bool canUse(const char* weaponName) const;

	virtual void chooseTarget();
};
