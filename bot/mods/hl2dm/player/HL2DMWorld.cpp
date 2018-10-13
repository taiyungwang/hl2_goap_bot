#include "HL2DMWorld.h"

#include <goap/WorldCond.h>
#include <player/Blackboard.h>
#include <player/Player.h>

void HL2DMWorld::addStates() {
	states.Insert(WorldProp::ARMOR_FULL, false);
}

bool HL2DMWorld::update(Blackboard& blackboard) {
	updateState(WorldProp::ARMOR_FULL, blackboard.getSelf()->getArmor() >= 100);
	extern IVEngineServer* engine;
	if (CUtlString("weapon_physcannon")
			== engine->PEntityOfEntIndex(blackboard.getArmory().getCurrWeaponIdx())->GetClassName()) {
		updateState(WorldProp::WEAPON_LOADED, true);
	}
	return false;
}
