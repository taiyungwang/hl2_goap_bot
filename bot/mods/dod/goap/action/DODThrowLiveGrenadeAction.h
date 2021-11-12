#pragma once

#include <goap/action/ThrowGrenadeAction.h>

class DODThrowLiveGrenadeAction: public ThrowGrenadeAction {
public:
	DODThrowLiveGrenadeAction(Blackboard &blackboard) :
		ThrowGrenadeAction(blackboard) {
		effects = { WorldProp::HAS_LIVE_GRENADE, false };
	}

private:
	virtual const Player *chooseTarget() const override;

	virtual bool canUse(const char *weaponName) const override;
};
