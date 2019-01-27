#include "World.h"

#include "Blackboard.h"
#include "Bot.h"
#include <weapon/Weapon.h>
#include <util/BaseEntity.h>
#include <eiface.h>

void World::reset() {
	states.RemoveAll();
	states.Insert(WorldProp::USING_DESIRED_WEAPON, false);
	states.Insert(WorldProp::ENEMY_SIGHTED, false);
	states.Insert(WorldProp::MULTIPLE_ENEMY_SIGHTED, false);
	states.Insert(WorldProp::HURT, false);
	states.Insert(WorldProp::IS_BLOCKED, false);
	states.Insert(WorldProp::USING_BEST_WEAP, false);
	states.Insert(WorldProp::WEAPON_LOADED, true);
	states.Insert(WorldProp::OUT_OF_AMMO, false);
	states.Insert(WorldProp::ROUND_STARTED, roundStarted);
	addStates();
}

bool World::think(Blackboard& blackboard) {
	bool& enemySighted = states[states.Find(WorldProp::ENEMY_SIGHTED)];
	bool noEnemy = !enemySighted;
	enemySighted = blackboard.getTargetedPlayer() != nullptr;
	updateState(WorldProp::MULTIPLE_ENEMY_SIGHTED,
			blackboard.getVisibleEnemies().Count() > 1);
	Armory& armory = blackboard.getArmory();
	updateState(WorldProp::USING_BEST_WEAP,
			armory.getBestWeaponIdx() == armory.getCurrWeaponIdx());
	edict_t* blocker = blackboard.getBlocker();
	if (blocker != nullptr) {
		if (BaseEntity(blocker).isDestroyedOrUsed()) {
			blackboard.setBlocker(nullptr);
		} else {
			extern IVEngineServer* engine;
			auto& players = blackboard.getPlayers();
			int entIndx = engine->IndexOfEdict(blocker);
			auto i = players.Find(entIndx);
			if (players.IsValidIndex(i) && players[i]->isDead()) {
				blackboard.setBlocker(nullptr);
			}
		}
	}
	auto& weapons = armory.getWeapons();
	int currentWeap = armory.getCurrWeaponIdx();
	if (currentWeap > 0) {
		Weapon *currWeap = weapons[weapons.Find(currentWeap)];
		updateState(WorldProp::WEAPON_LOADED,
				!currWeap->isClipEmpty());
		updateState(WorldProp::OUT_OF_AMMO,
				currWeap->isOutOfAmmo(blackboard.getSelf()->getEdict()));
	}
	updateState(WorldProp::HURT, false);
	updateState(WorldProp::IS_BLOCKED, blackboard.getBlocker() != nullptr);
	// reset planner if this is first time we see enemy.
	return update(blackboard) || (noEnemy && enemySighted)
			|| states[states.Find(WorldProp::HURT)];
}
