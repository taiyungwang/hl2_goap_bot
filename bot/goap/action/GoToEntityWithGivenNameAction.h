#pragma once

#include "GoToEntityAction.h"
#include <utllinkedlist.h>

/**
 * Action to get an item that is consumed.
 */
class GoToEntityWithGivenNameAction: public GoToEntityAction {
public:
	GoToEntityWithGivenNameAction(Blackboard& blackboard, const char* itemName);

	virtual bool execute();

protected:
	/**
	 * All instances of the specified entity class available
	 * for this map.
	 */
	CUtlLinkedList<edict_t*> items;

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

	/**
	 * Choose the item to go to.  Expected item
	 * to be set.
	 *
	 * @param active List of active items to be selected.
	 */
	virtual void selectFromActive(CUtlLinkedList<edict_t*>& active);
};
