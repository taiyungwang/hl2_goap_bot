#pragma once

#include "CapturePointAction.h"

class DODDefuseBombAction : public CapturePointAction {
public:
	DODDefuseBombAction(Blackboard& blackboard);

	virtual bool execute();

	virtual bool isInterruptable() const {
		return interruptable;
	}

	float getCost() {
		return GoToAction::getCost() + 2.0f;
	}

protected:
	bool interruptable = true;

	virtual bool isAvailable(const DODObjective& obj);

	virtual bool isTeammateActing(edict_t* teammate) const;

private:

	bool findTargetLoc();

	bool isAvailable(edict_t* ent);
};
