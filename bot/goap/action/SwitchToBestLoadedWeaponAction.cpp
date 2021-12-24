#include "SwitchToBestLoadedWeaponAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
#include <util/BasePlayer.h>
#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>

SwitchToBestLoadedWeaponAction::SwitchToBestLoadedWeaponAction(
		Blackboard& blackboard) :
		SwitchToDesiredWeaponAction(blackboard) {
	effects = {WorldProp::WEAPON_LOADED, true};
}

static bool ignore(const Weapon* weap, Blackboard& blackboard, float dist) {
	return weap->isClipEmpty() || !weap->isInRange(dist);
}

bool SwitchToBestLoadedWeaponAction::precondCheck() {
	if (!SwitchToDesiredWeaponAction::precondCheck()) {
		return false;
	}
	int best = arsenal.getBestWeapon(blackboard, ignore);
	if (best == 0 || best == arsenal.getCurrWeaponIdx()) {
		return false;
	}
	arsenal.setDesiredWeaponIdx(best);
	return true;
}
