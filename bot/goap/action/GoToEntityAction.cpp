#include "GoToEntityAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
#include <util/UtilTrace.h>
#include <edict.h>

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

void GoToEntityAction::setTargetLocAndRadius(edict_t* target) {
	auto collide = target->GetCollideable();
	targetLoc = collide->GetCollisionOrigin();
	Vector min, max;
	collide->WorldSpaceSurroundingBounds(&min, &max);
	if (min.LengthSqr() > 0.0f || max.LengthSqr() > 0.0f) {
		// look for trigger zone
		targetLoc = (min + max) / 2.0f;
		targetRadius = MIN(targetLoc.x - min.x, targetLoc.y - min.y);
	}
	targetLoc.z = UTIL_FindGround(targetLoc).z;
}


