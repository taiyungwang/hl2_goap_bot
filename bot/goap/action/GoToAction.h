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

	virtual void init();

	virtual bool precondCheck();

	virtual float getCost();

	virtual bool postCondCheck();

	virtual bool isInterruptable() const {
		return true;
	}

protected:
	CUtlStack<CNavArea*> path;

	Vector targetLoc;

	float targetRadius = 5.0f;

	GoToAction(Blackboard& blackboard): Action(blackboard) {
	}

	virtual bool findTargetLoc() = 0;
};
