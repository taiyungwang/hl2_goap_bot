#include "SwitchWeaponAction.h"

#include <weapon/Weapon.h>
#include <player/Blackboard.h>

SwitchWeaponAction::SwitchWeaponAction(Blackboard& blackboard) :
		WeaponAction(blackboard) {
	effects = {WorldProp::USING_BEST_WEAP, true};
}

bool SwitchWeaponAction::precondCheck() {
	return armory.getBestWeaponIdx() != 0;
}

bool SwitchWeaponAction::execute() {
	int idx = armory.getBestWeaponIdx();
	armory.setCurrentWeaponIdx(idx);
	blackboard.getCmd().weaponselect = idx;
	return true;
}
