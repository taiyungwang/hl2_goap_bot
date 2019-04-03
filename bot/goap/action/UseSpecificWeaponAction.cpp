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
		const char* name = Armory::getWeaponName(weapons.Key(i));
		if (name != nullptr && canUse(name)) {
			weapIdx = weapons.Key(i);
			if (armory.getWeapon(weapIdx) == nullptr) {
				return false;
			}
			armory.setDesiredWeaponIdx(weapIdx);
			return true;
		}
	}
	return false;
}
