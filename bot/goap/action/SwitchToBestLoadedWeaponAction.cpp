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
	int best = arsenal.getBestWeapon(blackboard, ignore);
	if (!SwitchToDesiredWeaponAction::precondCheck() || best == 0) {
		return false;
	}
	arsenal.setDesiredWeaponIdx(best);
	return true;
}
