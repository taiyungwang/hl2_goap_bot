#include "SwitchWeaponAction.h"

#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>
#include <player/Blackboard.h>

SwitchWeaponAction::SwitchWeaponAction(Blackboard& blackboard) :
	SwitchToDesiredWeaponAction(blackboard) {
	effects = {WorldProp::USING_BEST_WEAP, true};
}

bool SwitchWeaponAction::precondCheck() {
	if (!SwitchToDesiredWeaponAction::precondCheck()) {
		return false;
	}
	int best = arsenal.getBestWeapon(blackboard,
			[] (const Weapon*, Blackboard&, float) {
		return false;
	});
	if (best > 0 && best != arsenal.getCurrWeaponIdx()) {
		arsenal.setDesiredWeaponIdx(best);
		return true;
	}
	return false;
}
