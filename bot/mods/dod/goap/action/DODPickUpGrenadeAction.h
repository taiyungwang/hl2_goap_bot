#pragma once

#include <goap/action/Action.h>

class DODPickUpGrenadeAction: public Action {
public:
	DODPickUpGrenadeAction(Bot *self) :
			Action(self) {
		effects = { WorldProp::EXPLOSIVE_NEAR, false };
	}

	bool precondCheck() override;

	bool execute() override;
};
