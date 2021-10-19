#pragma once

#include "DODDefuseBombAction.h"

class DODBombTargetAction: public DODDefuseBombAction {
public:
	DODBombTargetAction(Blackboard& blackboard);

private:
	bool isAvailable(const DODObjective& obj);

	bool isTeammateActing(edict_t* teammate) const override;
};
