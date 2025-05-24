#include "UseSpecificWeaponAction.h"

#include <player/Bot.h>
#include <weapon/Weapon.h>
#include <eiface.h>

bool UseSpecificWeaponAction::precondCheck() {
	weapIdx = 0;
	self->forMyWeapons([this](edict_t *weaponEnt) mutable -> bool {
		extern IVEngineServer* engine;
		int i = engine->IndexOfEdict(weaponEnt);
		if (this->canUse(i)) {
			weapIdx = i;
			return true;
		}
		return false;
	});
	return weapIdx > 0;
}

bool UseSpecificWeaponAction::canUse(int i) const {
	return !self->getWeapon(i)->isOutOfAmmo(self->getEdict());
}

