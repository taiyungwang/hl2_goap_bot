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
	setTargetLocAndRadius(item);
	return true;
}

void GoToEntityAction::selectItem() {
	item = nullptr;
	selectFromActive(items);
}

void GoToEntityAction::setTargetLocAndRadius(edict_t* target) {
	auto collide = target->GetCollideable();
	targetLoc = collide->GetCollisionOrigin();
	Vector min, max;
	collide->WorldSpaceTriggerBounds(&min, &max);
	if (min.LengthSqr() > 0.0f || max.LengthSqr() > 0.0f) {
		// look for trigger zone
		targetLoc = (min + max) / 2.0f;
		targetLoc.z = min.z;
		targetRadius = MIN(targetLoc.x - min.x, targetLoc.y - min.y) - 1.0f;
	}
}

void GoToEntityAction::selectFromActive(CUtlLinkedList<edict_t*>& active) {
	item = findNearestEntity(active,
			blackboard.getSelf()->getCurrentPosition());
}

