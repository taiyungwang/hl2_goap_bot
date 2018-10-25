#pragma once

#include <goap/action/GoToConsumableEntityAction.h>

class EntityVar;

/**
 * Defines an action to pick up or use a consumable item on
 * the map. The availability of the item is defined by
 * properties of an entity.
 */
class GoToItemAction: public GoToConsumableEntityAction {
public:
	GoToItemAction(const char* name, const char* className,
			const char* classVar, Blackboard& blackboard);

protected:
	/**
	 * Checks the availability of the target item.
	 */
	EntityVar& availability;
};
