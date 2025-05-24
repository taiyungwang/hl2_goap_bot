#pragma once

#include "DODUseRifleGrenadeAction.h"

class Player;

class DODUseFragGrenadeAction final : public DODUseRifleGrenadeAction {
public:
	explicit DODUseFragGrenadeAction(Bot *self) :
			DODUseRifleGrenadeAction(self) {
	}

	bool init() override {
		primeDuration = 0;
		return true;
	}

private:
	int primeDuration = 0;

	bool use() override;

	bool canUse(const char* weaponName) const override;
};
