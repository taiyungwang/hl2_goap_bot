#include "SwitchToDesiredWeaponAction.h"

#include <player/Blackboard.h>

bool SwitchToDesiredWeaponAction::precondCheck() {
	return !blackboard.isOnLadder();
}

void SwitchToDesiredWeaponAction::init() {
	blackboard.getCmd().weaponselect = armory.getDesiredWeaponIdx();
}

bool SwitchToDesiredWeaponAction::execute() {
	return armory.getDesiredWeaponIdx() == armory.getCurrWeaponIdx();
}
