#include "SwitchToDesiredWeaponAction.h"

#include <player/Bot.h>

bool SwitchToDesiredWeaponAction::precondCheck() {
	return !self->isOnLadder();
}

bool SwitchToDesiredWeaponAction::execute() {
	int desiredWeapIdx = self->getDesiredWeapon();
	if (desiredWeapIdx == self->getCurrWeaponIdx()) {
		return true;
	}
	self->getCmd().weaponselect = desiredWeapIdx;
	return false;
}
