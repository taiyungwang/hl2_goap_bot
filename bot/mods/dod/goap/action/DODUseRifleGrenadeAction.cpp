#include "DODUseRifleGrenadeAction.h"

#include <player/Bot.h>
#include <weapon/Weapon.h>
#include <util/BaseGrenade.h>

bool DODUseRifleGrenadeAction::execute() {
	auto weapon = self->getCurrWeapon();
	return !weapon || weapon->isClipEmpty() || ThrowGrenadeAction::execute();
}

bool DODUseRifleGrenadeAction::canUse(const char* weaponName) const {
	return Q_strcmp(weaponName, "weapon_riflegren_us") == 0
			|| Q_strcmp(weaponName, "weapon_riflegren_ger") == 0;
}
