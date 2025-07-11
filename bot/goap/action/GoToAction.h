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

	virtual bool precondCheck() override = 0;

	virtual bool init();

	virtual float getCost();

	virtual bool goalComplete();

	virtual bool isInterruptable() const {
		return canAbort;
	}

protected:
	Vector targetLoc;

	bool sprint = false;

	float targetRadius = 0.0f;

	GoToAction(Bot *self): Action(self) {
	}

	/**
	 * Randomly choose a target based on its relative distance.
	 */
	edict_t* randomChoice(CUtlLinkedList<edict_t*>& targets);

private:
	bool canAbort = true;
};
