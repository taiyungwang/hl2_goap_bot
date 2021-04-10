#include "SwitchToBestLoadedWeaponAction.h"

#include <weapon/Armory.h>
#include <weapon/Weapon.h>
#include <player/Blackboard.h>
#include <player/Player.h>
#include <util/BasePlayer.h>

SwitchToBestLoadedWeaponAction::SwitchToBestLoadedWeaponAction(
		Blackboard& blackboard) :
		SwitchToDesiredWeaponAction(blackboard) {
	effects = {WorldProp::WEAPON_LOADED, true};
}

static bool ignore(const Weapon* weap, Blackboard& blackboard, float dist) {
	Armory& armory = blackboard.getArmory();
	auto& weapons = armory.getWeapons();
	return weap->isClipEmpty() || !weap->isInRange(dist);
}

bool SwitchToBestLoadedWeaponAction::precondCheck() {
	int best = armory.getBestWeapon(blackboard, ignore);
	if (best == 0) {
		return false;
	}
	armory.setDesiredWeaponIdx(best);
	return true;
}
