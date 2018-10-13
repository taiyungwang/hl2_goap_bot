#include <goap/action/ReloadWeaponAction.h>
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <in_buttons.h>

ReloadWeaponAction::ReloadWeaponAction(Blackboard& blackboard) :
		Action(blackboard) {
	effects = {WorldProp::WEAPON_LOADED, true};
	precond.Insert(WorldProp::OUT_OF_AMMO, false);
	precond.Insert(WorldProp::ENEMY_SIGHTED, false);
}

bool ReloadWeaponAction::execute() {
	blackboard.getButtons().tap(IN_RELOAD);
	return true;
}
