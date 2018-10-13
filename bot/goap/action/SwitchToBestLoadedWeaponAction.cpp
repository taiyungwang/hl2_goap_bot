#include "SwitchToBestLoadedWeaponAction.h"

#include <weapon/Armory.h>
#include <weapon/Weapon.h>
#include <player/Blackboard.h>
#include <player/Player.h>
#include <util/BasePlayer.h>

SwitchToBestLoadedWeaponAction::SwitchToBestLoadedWeaponAction(
		Blackboard& blackboard) :
		WeaponAction(blackboard) {
	effects = {WorldProp::WEAPON_LOADED, true};
}

static bool ignore(const Weapon* weap, Blackboard& blackboard, float dist) {
	Armory& armory = blackboard.getArmory();
	auto& weapons = armory.getWeapons();
	return weap->isClipEmpty()
			|| !weap->isInRange(dist)
			|| weap == armory.getCurrWeapon();
}

bool SwitchToBestLoadedWeaponAction::precondCheck() {
	nextBest = armory.getBestWeapon(blackboard, ignore);
	return nextBest != 0;
}

bool SwitchToBestLoadedWeaponAction::execute() {
	blackboard.getCmd().weaponselect = nextBest;
	armory.setCurrentWeaponIdx(nextBest);
	return true;
}
