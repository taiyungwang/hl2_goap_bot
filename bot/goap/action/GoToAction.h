#pragma once

#include "Action.h"
#include <utlstack.h>

class CNavArea;

/**
 * Defines the action for going to a location on the map.
 */
class GoToAction: public Action {
public:

	virtual bool execute();

	void init();

	virtual bool precondCheck();

	virtual float getCost() const {
		return path.Count();
	}

	virtual bool postCondCheck();

	virtual bool isInterruptable() const {
		return true;
	}

protected:
	CUtlStack<CNavArea*> path;

	Vector targetLoc;

	float targetRadius = 25.0f;

	GoToAction(Blackboard& blackboard): Action(blackboard) {
	}
};
