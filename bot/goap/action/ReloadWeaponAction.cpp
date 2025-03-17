#include "ReloadWeaponAction.h"

#include <player/Bot.h>
#include <player/Buttons.h>
#include <weapon/Reloader.h>
#include <weapon/Weapon.h>
#include <util/SimpleException.h>
#include <in_buttons.h>

ReloadWeaponAction::ReloadWeaponAction(Bot *self) :
		Action(self) {
	effects = {WorldProp::WEAPON_LOADED, true};
	precond[WorldProp::OUT_OF_AMMO] = false;
	precond[WorldProp::ENEMY_SIGHTED] = false;
}

bool ReloadWeaponAction::precondCheck() {
	return self->getVision().getVisibleEnemies().empty()
			&& !self->isOnLadder();
}

bool ReloadWeaponAction::execute() {
	auto weapon = self->getCurrWeapon();
	if (!weapon) {
		return true;
	}
	Reloader* reloader = weapon->getReloader();
	if (reloader == nullptr) {
		throw new SimpleException(CUtlString("Reloader not set for ")
				+ weapon->getEdict()->GetClassName());
	}
	return reloader->execute(self);
}
