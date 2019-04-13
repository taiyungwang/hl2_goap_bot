#include "GoToConsumableEntityAction.h"

#include <player/Blackboard.h>
#include <edict.h>

bool GoToConsumableEntityAction::execute() {
	if (!GoToEntityAction::execute()) {
		return false;
	}
	if (GoToAction::postCondCheck()
		&& isDepleted()
		&& depleted.Find(item) == depleted.InvalidIndex()) {
		depleted.AddToTail(item);
	}
	return true;
}

void GoToConsumableEntityAction::selectItem() {
	CUtlLinkedList<edict_t*> active;
	FOR_EACH_LL(items, i)
	{
		auto j = depleted.Find(items[i]);
		bool available = isAvailable(items[i]);
		bool isDepleted = depleted.IsValidIndex(j);
		if (!available && allItemsVisible && !isDepleted) {
			depleted.AddToTail(items[i]);
		}
		if ((!isDepleted && !allItemsVisible)
				|| (allItemsVisible && available)) {
			active.AddToTail(items[i]);
		}
		if (available && isDepleted) {
			depleted.Remove(j);
		}
	}
	item = nullptr;
	selectFromActive(active);
}

bool GoToConsumableEntityAction::isDepleted() {
	return !isAvailable(item);
}
