#pragma once

#include "DODUseSmokeGrenadeAction.h"

class DODUseRifleGrenadeAction: public DODUseSmokeGrenadeAction {
public:
	explicit DODUseRifleGrenadeAction(Bot *self) :
			DODUseSmokeGrenadeAction(self) {
	}

protected:
	bool canUse(const char* weaponName) const override;

	bool use() override;

private:
	const Player *chooseTarget() const override {
		return ThrowGrenadeAction::chooseTarget();
	}
};
