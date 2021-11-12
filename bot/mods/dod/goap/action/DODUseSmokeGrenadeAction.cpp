#include "DODUseSmokeGrenadeAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>

bool DODUseSmokeGrenadeAction::precondCheck() {
	auto self = blackboard.getSelf();
	auto vision = self->getVision();
	return !vision.getNearbyTeammates().empty()
			&& vision.getVisibleEnemies().size() > 1
			&& ThrowGrenadeAction::precondCheck();
}

bool DODUseSmokeGrenadeAction::canUse(const char* weaponName) const {
	return Q_strcmp(weaponName, "weapon_smoke_us") == 0
			|| Q_strcmp(weaponName, "weapon_smoke_ger") == 0;
}

