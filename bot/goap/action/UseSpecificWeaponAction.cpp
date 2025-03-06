#include "UseSpecificWeaponAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
#include <weapon/Weapon.h>
#include <eiface.h>

UseSpecificWeaponAction::UseSpecificWeaponAction(Blackboard& blackboard) : SwitchToDesiredWeaponAction(blackboard) {
	precond[WorldProp::USING_DESIRED_WEAPON] = true;
}

bool UseSpecificWeaponAction::precondCheck() {
	weapIdx = 0;
	blackboard.getSelf()->forMyWeapons([this](edict_t *weaponEnt) mutable -> bool {
		extern IVEngineServer* engine;
		int i = engine->IndexOfEdict(weaponEnt);
		if (this->canUse(i)) {
			weapIdx = i;
			blackboard.getSelf()->setDesiredWeapon(weapIdx);
			return true;
		}
		return false;
	});
	return weapIdx > 0;
}

bool UseSpecificWeaponAction::canUse(int i) const {
	return !blackboard.getSelf()->getWeapon(i)->isOutOfAmmo(blackboard.getSelf()->getEdict());
}
