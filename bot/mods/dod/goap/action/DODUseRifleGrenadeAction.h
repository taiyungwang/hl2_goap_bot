#pragma once

#include "DODUseSmokeGrenadeAction.h"

class DODUseRifleGrenadeAction: public DODUseSmokeGrenadeAction {
public:
	DODUseRifleGrenadeAction(Bot *self) :
			DODUseSmokeGrenadeAction(self) {
	}

	virtual bool execute() override;

protected:
	virtual bool canUse(const char* weaponName) const;

private:
	const Player *chooseTarget() const override {
		return ThrowGrenadeAction::chooseTarget();
	}
};
