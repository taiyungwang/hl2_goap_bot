#include "SwitchWeaponAction.h"

#include <weapon/Weapon.h>
#include <player/Blackboard.h>

SwitchWeaponAction::SwitchWeaponAction(Blackboard& blackboard) :
		WeaponAction(blackboard) {
	effects = {WorldProp::USING_BEST_WEAP, true};
}

bool SwitchWeaponAction::precondCheck() {
	return !blackboard.isOnLadder() && armory.getBestWeaponIdx() != 0
			&& armory.getCurrWeaponIdx() != armory.getBestWeaponIdx();
}

void SwitchWeaponAction::init() {
	blackboard.getCmd().weaponselect = armory.getBestWeaponIdx();
}

bool SwitchWeaponAction::execute() {
	return armory.getBestWeaponIdx() == armory.getCurrWeaponIdx();
}
