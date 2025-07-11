#include "DODUseSmokeGrenadeAction.h"

#include <player/Bot.h>

bool DODUseSmokeGrenadeAction::precondCheck() {
	return !self->getVision().getNearbyTeammates().empty()
			&& ThrowGrenadeAction::precondCheck();
}

bool DODUseSmokeGrenadeAction::canUse(const char* weaponName) const {
	return Q_strcmp(weaponName, "weapon_smoke_us") == 0
			|| Q_strcmp(weaponName, "weapon_smoke_ger") == 0;
}

