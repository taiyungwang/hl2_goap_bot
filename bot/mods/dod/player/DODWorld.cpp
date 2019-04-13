#include "DODWorld.h"

#include <event/EventInfo.h>
#include <weapon/Weapon.h>
#include <player/Blackboard.h>
#include <player/PlayerManager.h>
#include <player/Bot.h>
#include <utlstring.h>

void DODWorld::addStates() {
	states.Insert(WorldProp::ALL_POINTS_CAPTURED, false);
	states.Insert(WorldProp::POINTS_DEFENDED, false);
	states.Insert(WorldProp::HAS_BOMB, false);
	states.Insert(WorldProp::BOMB_DEFUSED, false);
}

bool DODWorld::handle(EventInfo* event) {
	CUtlString name(event->getName());
	bool bombPlanted = name == "dod_bomb_planted";
	if (name == "dod_point_captured" || bombPlanted
			|| name == "dod_bomb_exploded" || name == "dod_bomb_defused") {
		if (bombPlanted) {
			extern PlayerManager *playerManager;
			bombPlantTeam = playerManager->getPlayer(event->getInt("userid"))->getTeam();
		} else {
			bombPlantTeam = 1;
		}
		reset = true;
		return false;
	}
	if (name == "dod_round_active") {
		if (!roundStarted) {
			reset = true;
		}
		roundStarted = true;
		bombPlantTeam = 1;
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
		Weapon* currWeap = armory.getCurrWeapon();
	}
	bool hasBomb = false;
	FOR_EACH_MAP_FAST(weapons, i) {
		if (CUtlString("weapon_basebomb") == armory.getWeaponName(weapons.Key(i))) {
			hasBomb = true;
			break;
		}
	}
	updateState(WorldProp::HAS_BOMB, hasBomb);
	if (reset) {
		reset = false;
		return true;
	}
	return false;
}
