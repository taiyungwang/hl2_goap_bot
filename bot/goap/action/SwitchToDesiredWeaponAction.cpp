#include "SwitchToDesiredWeaponAction.h"

#include <player/Blackboard.h>
#include <player/Player.h>
#include <weapon/Weapon.h>

bool SwitchToDesiredWeaponAction::precondCheck() {
	auto idx = armory.getDesiredWeaponIdx();
	return idx != 0 && armory.getWeapon(idx) != nullptr;
}

bool SwitchToDesiredWeaponAction::execute() {
	blackboard.getCmd().weaponselect = armory.getDesiredWeaponIdx();
	return true;
}
