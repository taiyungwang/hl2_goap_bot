#include "ChargeAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
#include <nav_mesh/nav.h>
#include <util/EntityVar.h>
#include <util/UtilTrace.h>
#include <edict.h>
#include <in_buttons.h>

bool ChargeAction::precondCheck() {
	chargeTime = 0;
	return true;
}

bool ChargeAction::execute() {
	if (!GoToItemAction::execute()) {
		return false;
	}
	if (!GoToAction::goalComplete() || chargeTime++ > 900 || isFinished()) {
		return true;
	}
	if (isDepleted()) {
		if (depleted.Find(item) == depleted.InvalidIndex()) {
			depleted.AddToTail(item);
		}
		return true;
	}
	Vector itemPos = UTIL_FindGround(
			item->GetCollideable()->GetCollisionOrigin());
	itemPos.z += HumanHeight - 10.0f;
	blackboard.getButtons().hold(IN_USE);
	blackboard.getSelf()->setViewTarget(itemPos);
	return false;
}

bool ChargeAction::isAvailable(edict_t* ent) {
	return availability.get<float>(ent) < 1.0f;
}

bool ChargeAction::isDepleted() {
	return availability.get<float>(item) == 1.0f;
}

bool ChargeAction::findTargetLoc() {
	if (!GoToEntityAction::findTargetLoc()) {
		return false;
	}
	targetRadius = 15.0f;
	// override the trigger distance
	return true;
}
