#include "SwitchToDesiredWeaponAction.h"

#include <player/Bot.h>

bool SwitchToDesiredWeaponAction::precondCheck() {
	return !self->isOnLadder();
}

bool SwitchToDesiredWeaponAction::execute() {
	if (self->getDesiredWeapon() != weapIdx) {
		self->setDesiredWeapon(weapIdx);
		return false;
	}
	return self->getDesiredWeapon() == self->getCurrWeaponIdx();
}
