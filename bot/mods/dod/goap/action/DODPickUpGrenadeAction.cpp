#include "DODPickUpGrenadeAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
#include <player/Buttons.h>
#include <util/BaseGrenade.h>
#include <in_buttons.h>

bool DODPickUpGrenadeAction::precondCheck() {
	auto self = blackboard.getSelf();
	Vector pos = self->getCurrentPosition();
	for (auto i: self->getVision().getVisibleEntities()) {
		extern IVEngineServer *engine;
		edict_t *entity = engine->PEntityOfEntIndex(i);
		if (entity != nullptr && !entity->IsFree() && BaseGrenade(entity).getThrower() != self->getEdict()
				&& entity->GetCollideable()->GetCollisionOrigin().DistTo(pos) < 60.0f) {
			return true;
		}
	}
	return false;
}

bool DODPickUpGrenadeAction::execute() {
	if (!precondCheck()) {
		return true;
	}
	blackboard.getButtons().tap(IN_USE);
	return false;
}
