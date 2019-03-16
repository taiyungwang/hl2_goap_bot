#include "GoToConsumableEntityAction.h"

#include <edict.h>

bool GoToConsumableEntityAction::execute() {
	if (!GoToEntityAction::execute()) {
		return false;
	}
	if (isDepleted() && depleted.Find(item) == depleted.InvalidIndex()) {
		depleted.AddToTail(item);
	}
	return true;
}
bool GoToConsumableEntityAction::postCondCheck() {
	if (!GoToEntityAction::postCondCheck()) {
		return false;
	}
	if (item != nullptr && isDepleted()) {
		depleted.AddToTail(item);
	}
	return true;
}

bool GoToConsumableEntityAction::precondCheck() {
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
	selectItem(active);
	return buildPathToEntity();
}
