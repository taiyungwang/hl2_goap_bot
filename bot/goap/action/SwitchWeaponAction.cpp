#include "SwitchWeaponAction.h"

#include <weapon/Weapon.h>
#include <player/Bot.h>
#include <player/Blackboard.h>

SwitchWeaponAction::SwitchWeaponAction(Blackboard& blackboard) :
	SwitchToDesiredWeaponAction(blackboard) {
	effects = {WorldProp::USING_BEST_WEAP, true};
}

bool SwitchWeaponAction::precondCheck() {
	if (!SwitchToDesiredWeaponAction::precondCheck()) {
		return false;
	}
	Bot *self = blackboard.getSelf();
	int best = self->getBestWeapon();
	if (best > 0 && best != self->getCurrWeaponIdx()) {
		self->setDesiredWeapon(best);
		return true;
	}
	return false;
}
