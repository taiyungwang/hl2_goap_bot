#pragma once

#include "DODDefendPointAction.h"

class DODDefuseBombAction : public DODDefendPointAction {
public:
	DODDefuseBombAction(Blackboard& blackboard);

	virtual bool execute();

protected:
	virtual bool isAvailable(int idx) const;
};
