#pragma once

#include <goap/action/UseSpecificWeaponAction.h>

class DODUseSmokeGrenadeAction: public UseSpecificWeaponAction {
public:
	DODUseSmokeGrenadeAction(Blackboard& blackboard);

	virtual bool precondCheck();

	virtual bool execute();

protected:
	virtual bool canUse(const char* weaponName) const;
};
