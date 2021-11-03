#include "SwitchWeaponAction.h"

#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>
#include <player/Blackboard.h>

SwitchWeaponAction::SwitchWeaponAction(Blackboard& blackboard) :
		WeaponAction(blackboard) {
	effects = {WorldProp::USING_BEST_WEAP, true};
}

bool SwitchWeaponAction::precondCheck() {
	return !blackboard.isOnLadder() && arsenal.getBestWeaponIdx() != 0
			&& arsenal.getCurrWeaponIdx() != arsenal.getBestWeaponIdx();
}

bool SwitchWeaponAction::execute() {
	if (arsenal.getBestWeaponIdx() == arsenal.getCurrWeaponIdx()) {
		return true;
	}
	blackboard.getCmd().weaponselect = arsenal.getBestWeaponIdx();
	return false;
}
