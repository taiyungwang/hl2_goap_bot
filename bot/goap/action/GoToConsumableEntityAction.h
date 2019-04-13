#pragma once

#include "GoToEntityAction.h"

/**
 * Action to get an item that is consumed.
 */
class GoToConsumableEntityAction: public GoToEntityAction {
public:
	GoToConsumableEntityAction(Blackboard& blackboard, const char* itemName) :
			GoToEntityAction(blackboard, itemName) {
	}

	virtual bool execute();

protected:
	/**
	 * All items that are used.
	 */
	CUtlLinkedList<edict_t*> depleted;

	bool allItemsVisible = false;

	/**
	 * Checks to see if this entity is available.
	 */
	virtual bool isAvailable(edict_t* ent) = 0;

	/**
	 * Check to see if current entity is depleted
	 */
	virtual bool isDepleted();

	virtual void selectItem();
};
