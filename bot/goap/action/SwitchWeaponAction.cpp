#include "SwitchWeaponAction.h"

#include <weapon/Weapon.h>
#include <player/Bot.h>

SwitchWeaponAction::SwitchWeaponAction(Bot *self) :
	SwitchToDesiredWeaponAction(self) {
	effects = {WorldProp::USING_BEST_WEAP, true};
}

bool SwitchWeaponAction::precondCheck() {
	if (!SwitchToDesiredWeaponAction::precondCheck()) {
		return false;
	}
	int best = self->getBestWeapon();
	if (best > 0 && best != self->getCurrWeaponIdx()) {
		self->setDesiredWeapon(best);
		return true;
	}
	return false;
}
