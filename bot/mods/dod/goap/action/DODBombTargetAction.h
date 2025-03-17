#pragma once

#include "DODDefuseBombAction.h"

class DODBombTargetAction: public DODDefuseBombAction {
public:
	DODBombTargetAction(Bot *self);

private:
	bool isAvailable(const DODObjective& obj);

	bool isActingOnBomb(edict_t* player) const override;
};
