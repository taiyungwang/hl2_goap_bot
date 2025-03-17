#include "SwitchToBestLoadedWeaponAction.h"

#include <player/Bot.h>
#include <util/BasePlayer.h>
#include <weapon/Weapon.h>

SwitchToBestLoadedWeaponAction::SwitchToBestLoadedWeaponAction(
		Bot *self) :
		SwitchToDesiredWeaponAction(self) {
	effects = {WorldProp::WEAPON_LOADED, true};
}

bool SwitchToBestLoadedWeaponAction::precondCheck() {
	if (!SwitchToDesiredWeaponAction::precondCheck()) {
		return false;
	}
	int best = self->getBestWeapon();
	if (best == 0 || best == self->getCurrWeaponIdx()) {
		return false;
	}
	self->setDesiredWeapon(best);
	return true;
}
