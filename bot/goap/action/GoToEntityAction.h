#pragma once

#include "FindPathAction.h"
#include <utllinkedlist.h>

class EntityVar;
struct edict_t;

/**
 * Defines an action to go to a class of entity on the map.
 */
class GoToEntityAction: public FindPathAction {
public:
	GoToEntityAction(Blackboard& blackboard, const char* itemName);

	virtual void init();

	virtual bool execute();

	virtual bool precondCheck();

	bool postCondCheck();

protected:
	/**
	 * All instances of the specified entity class available
	 * for this map.
	 */
	CUtlLinkedList<edict_t*> items;
	/**
	 * Checks to see if this entity is available.
	 */
	virtual bool isAvailable(edict_t* ent) const = 0;

	/**
	 * Check to see if current entity is depleted
	 */
	virtual bool isDepleted() const = 0;

	/**
	 * Choose the item to go to.  Expected item
	 * to be set.
	 *
	 * @param active List of active items to be selected.
	 */
	virtual void selectItem(CUtlLinkedList<edict_t*>& active);

	/**
	 * All items that are used.
	 */
	CUtlLinkedList<edict_t*> depleted;

	/**
	 * Target item.
	 */
	edict_t* item = nullptr;

	bool allItemsVisible = false;
};
