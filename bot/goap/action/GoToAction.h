#pragma once

#include "Action.h"
#include <utllinkedlist.h>

class CNavArea;
struct edict_t;

/**
 * Defines the action for going to a location on the map.
 */
class GoToAction: public Action {
public:

	virtual bool execute();

	virtual void init();

	virtual float getCost();

	virtual bool goalComplete();

	virtual bool isInterruptable() const {
		return canAbort;
	}

protected:
	Vector targetLoc;

	float targetRadius = 5.0f;

	GoToAction(Blackboard& blackboard): Action(blackboard) {
	}

	virtual bool findTargetLoc() = 0;

	/**
	 * Randomly choose a target based on its relative distance.
	 */
	edict_t* randomChoice(CUtlLinkedList<edict_t*>& targets);

private:
	bool canAbort = true;
};
