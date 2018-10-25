#include "GoToEntityAction.h"

#include <player/Blackboard.h>
#include <player/Player.h>
#include <util/EntityUtils.h>
#include <edict.h>

GoToEntityAction::GoToEntityAction(Blackboard& blackboard, const char* itemName) :
GoToAction(blackboard) {
	findEntWithMatchingName(itemName, items);
}

bool GoToEntityAction::precondCheck() {
	selectItem(items);
	return buildPathToEntity();
}

bool GoToEntityAction::postCondCheck() {
	item = nullptr;
	return GoToAction::postCondCheck();
}

bool GoToEntityAction::buildPathToEntity() {
	bool foundItem = item != nullptr;
	if (foundItem) {
		targetLoc = item->GetCollideable()->GetCollisionOrigin();
		if (targetLoc.x == 0 && targetLoc.y == 0 && targetLoc.z == 0) {
			// look for trigger zone
			Vector min, max;
			item->GetCollideable()->WorldSpaceTriggerBounds(&min, &max);
			targetLoc = (min + max) / 2.0f;
			targetLoc.z = min.z;
		}
	}
	return foundItem && GoToAction::precondCheck();
}

void GoToEntityAction::selectItem(CUtlLinkedList<edict_t*>& active) {
	item = findNearestEntity(active,
			blackboard.getSelf()->getCurrentPosition());
}

