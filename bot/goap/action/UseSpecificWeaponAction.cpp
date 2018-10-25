#include "UseSpecificWeaponAction.h"

#include <player/Blackboard.h>
#include <weapon/Weapon.h>

UseSpecificWeaponAction::UseSpecificWeaponAction(Blackboard& blackboard) :
		WeaponAction(blackboard) {
	precond.Insert(WorldProp::USING_DESIRED_WEAPON, true);
}

bool UseSpecificWeaponAction::precondCheck() {
	auto& weapons = armory.getWeapons();
	weapIdx = 0;
	FOR_EACH_MAP_FAST(weapons, i) {
		if (canUse(armory.getWeaponName(i))) {
			weapIdx = weapons.Key(i);
			return armory.getWeapon(weapIdx) != nullptr;
		}
	}
	return false;
}

void UseSpecificWeaponAction::init() {
	armory.setDesiredWeaponIdx(weapIdx);
}
