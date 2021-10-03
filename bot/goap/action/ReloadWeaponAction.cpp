#include "ReloadWeaponAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
#include <player/Buttons.h>
#include <weapon/Reloader.h>
#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>
#include <util/SimpleException.h>
#include <in_buttons.h>

ReloadWeaponAction::ReloadWeaponAction(Blackboard& blackboard) :
		Action(blackboard) {
	effects = {WorldProp::WEAPON_LOADED, true};
	precond.Insert(WorldProp::OUT_OF_AMMO, false);
	precond.Insert(WorldProp::ENEMY_SIGHTED, false);
}

bool ReloadWeaponAction::precondCheck() {
	return !blackboard.isOnLadder();
}

bool ReloadWeaponAction::execute() {
	Weapon* weapon = blackboard.getSelf()->getArsenal().getCurrWeapon();
	if (weapon == nullptr) {
		return true;
	}
	Reloader* reloader = weapon->getReloader();
	if (reloader == nullptr) {
		throw new SimpleException(CUtlString("Reloader not set for ")
				+ weapon->getEdict()->GetClassName());
	}
	return reloader->execute(blackboard);
}
