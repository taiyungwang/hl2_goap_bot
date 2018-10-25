#pragma once

#include "GoToAction.h"
#include <utllinkedlist.h>

class EntityVar;
struct edict_t;

/**
 * Defines an action to go to a class of entity on the map.
 */
class GoToEntityAction: public GoToAction {
public:

	virtual bool precondCheck();

	bool postCondCheck();

protected:
	GoToEntityAction(Blackboard& blackboard, const char* itemName);

	/**
	 * All instances of the specified entity class available
	 * for this map.
	 */
	CUtlLinkedList<edict_t*> items;

	/**
	 * Choose the item to go to.  Expected item
	 * to be set.
	 *
	 * @param active List of active items to be selected.
	 */
	virtual void selectItem(CUtlLinkedList<edict_t*>& active);

	bool buildPathToEntity();
	/**
	 * Target item.
	 */
	edict_t* item = nullptr;
};
