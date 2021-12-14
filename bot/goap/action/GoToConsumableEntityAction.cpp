#include "GoToConsumableEntityAction.h"
#include <player/Bot.h>
#include <player/Blackboard.h>
#include <util/EntityUtils.h>
#include <edict.h>

GoToConsumableEntityAction::GoToConsumableEntityAction(Blackboard &blackboard,
		const char *itemName) :
		GoToEntityAction(blackboard) {
	findEntWithMatchingName(itemName, items);

}

bool GoToConsumableEntityAction::execute() {
	if (!GoToEntityAction::execute()) {
		return false;
	}
	if (GoToAction::goalComplete()
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

void GoToConsumableEntityAction::selectFromActive(CUtlLinkedList<edict_t*>& active) {
	item = findNearestEntity(active,
			blackboard.getSelf()->getCurrentPosition());
}
