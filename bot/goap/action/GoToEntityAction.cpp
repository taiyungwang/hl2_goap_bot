#include "GoToEntityAction.h"

#include <player/Blackboard.h>
#include <player/Player.h>
#include <util/EntityUtils.h>
#include <util/EntityClassManager.h>
#include <util/EntityClass.h>
#include <util/EntityVar.h>
#include <edict.h>
#include <in_buttons.h>

GoToEntityAction::GoToEntityAction(Blackboard& blackboard, const char* itemName) :
FindPathAction(blackboard) {
	findEntWithSubStrInName(itemName, items);
	precond.Insert(WorldProp::AT_LOCATION, true);
}

void GoToEntityAction::init() {
	blackboard.setTargetRadius(25.0f);
	FindPathAction::init();
}

bool GoToEntityAction::execute() {
	if (item == nullptr) {
		return true;
	}
	if (isDepleted()) {
		depleted.AddToTail(item);
		return true;
	}
	return false;
}

bool GoToEntityAction::postCondCheck() {
	item = nullptr;
	return true;
}

bool GoToEntityAction::precondCheck() {
	CUtlLinkedList<edict_t*> active;
	FOR_EACH_LL(items, i)
	{
		auto j = depleted.Find(items[i]);
		bool available = isAvailable(items[i]);
		if (!available && allItemsVisible) {
			depleted.AddToTail(items[i]);
		}
		if ((!depleted.IsValidIndex(j) && !allItemsVisible)
				|| (allItemsVisible && available)) {
			active.AddToTail(items[i]);
		}
		if (available && depleted.IsValidIndex(j)) {
			depleted.Remove(j);
		}
	}
	selectItem(active);
	bool foundItem = item != nullptr;
	if (foundItem) {
		targetLoc = item->GetCollideable()->GetCollisionOrigin();
		build();
	}
	return foundItem && path.Count() > 0;
}

void GoToEntityAction::selectItem(CUtlLinkedList<edict_t*>& active) {
	item = findNearestEntity(active,
			blackboard.getSelf()->getCurrentPosition());
}

