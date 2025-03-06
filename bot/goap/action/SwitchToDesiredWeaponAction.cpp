#include "SwitchToDesiredWeaponAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>

bool SwitchToDesiredWeaponAction::precondCheck() {
	return !blackboard.getSelf()->isOnLadder();
}

bool SwitchToDesiredWeaponAction::execute() {
	int desiredWeapIdx = blackboard.getSelf()->getDesiredWeapon();
	if (desiredWeapIdx == blackboard.getSelf()->getCurrWeaponIdx()) {
		return true;
	}
	blackboard.getCmd().weaponselect = desiredWeapIdx;
	return false;
}
