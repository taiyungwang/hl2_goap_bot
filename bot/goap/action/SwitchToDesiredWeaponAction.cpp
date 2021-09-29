#include "SwitchToDesiredWeaponAction.h"

#include <player/Blackboard.h>
#include <weapon/Arsenal.h>

bool SwitchToDesiredWeaponAction::precondCheck() {
	return !blackboard.isOnLadder();
}

void SwitchToDesiredWeaponAction::init() {
	blackboard.getCmd().weaponselect = arsenal.getDesiredWeaponIdx();
}

bool SwitchToDesiredWeaponAction::execute() {
	return arsenal.getDesiredWeaponIdx() == arsenal.getCurrWeaponIdx();
}
