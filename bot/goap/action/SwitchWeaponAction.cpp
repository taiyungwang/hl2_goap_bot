#include "SwitchWeaponAction.h"

#include <player/Bot.h>

bool SwitchWeaponAction::precondCheck() {
	if (!SwitchToDesiredWeaponAction::precondCheck()) {
		return false;
	}
	weapIdx = self->getBestWeapon();
	return weapIdx > 0;	
}
