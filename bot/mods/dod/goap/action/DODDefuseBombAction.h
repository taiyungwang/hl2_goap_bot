#pragma once

#include "DODDefendPointAction.h"

class DODDefuseBombAction : public DODDefendPointAction {
public:
	DODDefuseBombAction(Blackboard& blackboard);

	virtual bool execute();

	virtual bool isInterruptable() const {
		return interruptable;
	}

protected:
	virtual bool isAvailable(int idx) const;

	bool interruptable = true;
};
