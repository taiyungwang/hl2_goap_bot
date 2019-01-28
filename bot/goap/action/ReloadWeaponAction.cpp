#include "ReloadWeaponAction.h"

#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <weapon/Reloader.h>
#include <weapon/Weapon.h>
#include <in_buttons.h>

ReloadWeaponAction::ReloadWeaponAction(Blackboard& blackboard) :
		Action(blackboard) {
	effects = {WorldProp::WEAPON_LOADED, true};
	precond.Insert(WorldProp::OUT_OF_AMMO, false);
	precond.Insert(WorldProp::ENEMY_SIGHTED, false);
}

bool ReloadWeaponAction::execute() {
	Reloader* reloader = blackboard.getArmory().getCurrWeapon()->getReloader();
	return reloader == nullptr || reloader->execute(blackboard);
}
