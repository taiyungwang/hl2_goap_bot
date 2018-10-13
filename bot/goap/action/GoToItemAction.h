#pragma once

#include "GoToEntityAction.h"

class EntityVar;
struct edict_t;

/**
 * Defines an action to pick up or use a consumable item on
 * the map.
 */
class GoToItemAction: public GoToEntityAction {
public:
	GoToItemAction(const char* name, const char* className,
			const char* classVar, Blackboard& blackboard);

protected:
	/**
	 * Checks the availability of the target item.
	 */
	EntityVar& availability;
};
