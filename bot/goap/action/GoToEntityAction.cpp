#include "GoToEntityAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
#include <util/EntityUtils.h>
#include <edict.h>

GoToEntityAction::GoToEntityAction(Blackboard& blackboard, const char* itemName) :
GoToAction(blackboard) {
	findEntWithMatchingName(itemName, items);
}

void GoToEntityAction::init() {
	GoToAction::init();
	blackboard.setTarget(item);
}

bool GoToEntityAction::findTargetLoc() {
	selectItem();
	if (item == nullptr) {
		return false;
	}
	auto collide = item->GetCollideable();
	targetLoc = collide->GetCollisionOrigin();
	Vector min, max;
	collide->WorldSpaceTriggerBounds(&min, &max);
	if (targetLoc.x == 0 && targetLoc.y == 0 && targetLoc.z == 0) {
		// look for trigger zone
		targetLoc = (min + max) / 2.0f;
		targetLoc.z = min.z;
	}
	if (min.DistTo(max) > 0.0f) {
		max.z = min.z;
		targetRadius = max.DistTo(min) / 2.0f;
	}
	return true;
}

void GoToEntityAction::selectItem(CUtlLinkedList<edict_t*>& active) {
	item = findNearestEntity(active,
			blackboard.getSelf()->getCurrentPosition());
}

