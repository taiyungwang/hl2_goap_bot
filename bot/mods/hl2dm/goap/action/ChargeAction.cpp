#include "ChargeAction.h"

#include <player/Blackboard.h>
#include <navmesh/nav.h>
#include <util/EntityVar.h>
#include <util/UtilTrace.h>
#include <edict.h>
#include <in_buttons.h>


bool ChargeAction::execute() {
	if (!GoToItemAction::execute()) {
		return false;
	}
	if (isDepleted() || isFinished()) {
		return true;
	}
	Vector itemPos = UTIL_FindGround(
			item->GetCollideable()->GetCollisionOrigin());
	itemPos.z += HumanHeight - 10.0f;
	if (blackboard.getAimAccuracy(itemPos) > 0.96f) {
		blackboard.getButtons().hold(IN_USE);
	}
	blackboard.setViewTarget(itemPos);
	return false;
}

bool ChargeAction::isAvailable(edict_t* ent) const {
	return availability.get<float>(ent) < 1.0f;
}

bool ChargeAction::isDepleted() const {
	return availability.get<float>(item) == 1.0f;
}
