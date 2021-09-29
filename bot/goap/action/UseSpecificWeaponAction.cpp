#include "UseSpecificWeaponAction.h"

#include <player/Blackboard.h>
#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>

UseSpecificWeaponAction::UseSpecificWeaponAction(Blackboard& blackboard) :
		WeaponAction(blackboard) {
	precond.Insert(WorldProp::USING_DESIRED_WEAPON, true);
}

bool UseSpecificWeaponAction::precondCheck() {
	auto& weapons = arsenal.getWeapons();
	weapIdx = 0;
	FOR_EACH_MAP_FAST(weapons, i) {
		const char* name = Arsenal::getWeaponName(weapons.Key(i));
		if (name != nullptr && canUse(name)) {
			weapIdx = weapons.Key(i);
			if (arsenal.getWeapon(weapIdx) == nullptr) {
				return false;
			}
			arsenal.setDesiredWeaponIdx(weapIdx);
			return true;
		}
	}
	return false;
}
