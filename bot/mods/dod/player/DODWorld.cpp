#include "DODWorld.h"

#include <event/EventInfo.h>
#include <weapon/Weapon.h>
#include <player/Blackboard.h>
#include <utlstring.h>

void DODWorld::addStates() {
	states.Insert(WorldProp::ALL_POINTS_CAPTURED, false);
}

bool DODWorld::handle(EventInfo* event) {
	CUtlString name(event->getName());
	if (name == "dod_point_captured") {
		reset = true;
		return false;
	}
	if (name == "dod_round_active") {
		if (!roundStarted) {
			reset = true;
		}
		roundStarted = true;
	} else if (name == "dod_round_win" || name == "dod_game_over") {
		roundStarted = false;
	}
	updateState(WorldProp::ROUND_STARTED, roundStarted);
	return false;
}

bool DODWorld::update(Blackboard& blackboard) {
	Armory& armory = blackboard.getArmory();
	auto& weapons = armory.getWeapons();
	int currentWeap = armory.getCurrWeaponIdx();
	if (currentWeap > 0) {
		Weapon* currWeap = weapons[weapons.Find(currentWeap)];
		updateState(WorldProp::NEED_TO_DEPLOY_WEAPON, currWeap->isDeployable()
			&& currWeap->getMinDeployRange() < 0.0f && !currWeap->isDeployed());
	}
	if (reset) {
		reset = false;
		return true;
	}
	return false;
}
