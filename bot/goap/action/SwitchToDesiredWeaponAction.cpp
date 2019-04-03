#include "SwitchToDesiredWeaponAction.h"

#include <player/Blackboard.h>

bool SwitchToDesiredWeaponAction::execute() {
	blackboard.getCmd().weaponselect = armory.getDesiredWeaponIdx();
	return true;
}
