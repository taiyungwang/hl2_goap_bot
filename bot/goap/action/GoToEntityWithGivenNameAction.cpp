#include "GoToEntityWithGivenNameAction.h"
#include <player/Bot.h>
#include <util/EntityUtils.h>
#include <edict.h>

GoToEntityWithGivenNameAction::GoToEntityWithGivenNameAction(Bot *self,
		const char *itemName) :
		GoToEntityAction(self) {
	findEntWithMatchingName(itemName, items);

}

bool GoToEntityWithGivenNameAction::execute() {
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

void GoToEntityWithGivenNameAction::selectItem() {
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

bool GoToEntityWithGivenNameAction::isDepleted() {
	return !isAvailable(item);
}

void GoToEntityWithGivenNameAction::selectFromActive(CUtlLinkedList<edict_t*>& active) {
	item = findNearestEntity(active,
			self->getCurrentPosition());
}
