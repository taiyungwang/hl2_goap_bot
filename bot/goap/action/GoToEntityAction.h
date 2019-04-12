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
	void init();

protected:
	edict_t* item = nullptr;

	GoToEntityAction(Blackboard& blackboard, const char* itemName);

	/**
	 * All instances of the specified entity class available
	 * for this map.
	 */
	CUtlLinkedList<edict_t*> items;

	virtual void selectItem();

	void setTargetLocAndRadius(edict_t* target);

	/**
	 * Choose the item to go to.  Expected item
	 * to be set.
	 *
	 * @param active List of active items to be selected.
	 */
	virtual void selectFromActive(CUtlLinkedList<edict_t*>& active);

	virtual bool findTargetLoc();
};
