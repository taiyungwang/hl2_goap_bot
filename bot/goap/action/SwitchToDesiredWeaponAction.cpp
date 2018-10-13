#include "SwitchToDesiredWeaponAction.h"

#include <player/Blackboard.h>
#include <player/Player.h>
#include <weapon/Weapon.h>

bool SwitchToDesiredWeaponAction::precondCheck() {
	return armory.getDesiredWeaponIdx() != 0;
}

bool SwitchToDesiredWeaponAction::execute() {
	blackboard.getCmd().weaponselect = armory.getDesiredWeaponIdx();
	return true;
}
