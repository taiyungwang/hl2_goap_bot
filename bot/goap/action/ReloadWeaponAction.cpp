#include "ReloadWeaponAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
#include <player/Buttons.h>
#include <weapon/Reloader.h>
#include <weapon/Weapon.h>
#include <util/SimpleException.h>
#include <in_buttons.h>

ReloadWeaponAction::ReloadWeaponAction(Blackboard& blackboard) :
		Action(blackboard) {
	effects = {WorldProp::WEAPON_LOADED, true};
	precond[WorldProp::OUT_OF_AMMO] = false;
	precond[WorldProp::ENEMY_SIGHTED] = false;
}

bool ReloadWeaponAction::precondCheck() {
	return blackboard.getSelf()->getVision().getVisibleEnemies().empty()
			&& !blackboard.getSelf()->isOnLadder();
}

bool ReloadWeaponAction::execute() {
	auto weapon = blackboard.getSelf()->getCurrWeapon();
	if (!weapon) {
		return true;
	}
	Reloader* reloader = weapon->getReloader();
	if (reloader == nullptr) {
		throw new SimpleException(CUtlString("Reloader not set for ")
				+ weapon->getEdict()->GetClassName());
	}
	return reloader->execute(blackboard);
}
