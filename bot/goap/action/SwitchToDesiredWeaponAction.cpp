#include "SwitchToDesiredWeaponAction.h"

#include <player/Blackboard.h>
#include <weapon/Arsenal.h>

bool SwitchToDesiredWeaponAction::precondCheck() {
	return !blackboard.isOnLadder();
}

bool SwitchToDesiredWeaponAction::execute() {
	if (arsenal.getDesiredWeaponIdx() == arsenal.getCurrWeaponIdx()) {
		return true;
	}
	blackboard.getCmd().weaponselect = arsenal.getDesiredWeaponIdx();
	return false;
}
