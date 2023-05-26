#pragma once

#include "DODDefuseBombAction.h"

class DODBombTargetAction: public DODDefuseBombAction {
public:
	DODBombTargetAction(Blackboard& blackboard);

private:
	bool isAvailable(const DODObjective& obj);

	bool isActingOnBomb(edict_t* player) const override;
};
