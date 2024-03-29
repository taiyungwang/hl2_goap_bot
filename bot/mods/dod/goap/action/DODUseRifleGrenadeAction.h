#pragma once

#include "DODUseSmokeGrenadeAction.h"

class DODUseRifleGrenadeAction: public DODUseSmokeGrenadeAction {
public:
	DODUseRifleGrenadeAction(Blackboard& blackboard) :
			DODUseSmokeGrenadeAction(blackboard) {
	}

	virtual bool execute() override;

protected:
	virtual bool canUse(const char* weaponName) const;

private:
	const Player *chooseTarget() const override {
		return ThrowGrenadeAction::chooseTarget();
	}
};
