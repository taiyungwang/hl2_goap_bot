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
	states.Insert(WorldProp::HEALTH_FULL, true);
	states.Insert(WorldProp::IS_BLOCKED, false);
	states.Insert(WorldProp::USING_BEST_WEAP, false);
	states.Insert(WorldProp::WEAPON_LOADED, true);
	states.Insert(WorldProp::OUT_OF_AMMO, false);
	states.Insert(WorldProp::WEAPON_IN_RANGE, false);
	states.Insert(WorldProp::ROUND_STARTED, roundStarted);
	addStates();
}

bool World::think(Blackboard& blackboard) {
	bool& enemySighted = states[states.Find(WorldProp::ENEMY_SIGHTED)];
	updateState(WorldProp::MULTIPLE_ENEMY_SIGHTED,
			blackboard.getVisibleEnemies().Count() > 1);
	Armory& armory = blackboard.getArmory();
	updateState(WorldProp::USING_BEST_WEAP,
			armory.getBestWeaponIdx() == armory.getCurrWeaponIdx());
	edict_t* blocker = blackboard.getBlocker();
	bool inRange = true;
	const Player* enemy = blackboard.getTargetedPlayer();
	const Bot* self = blackboard.getSelf();
	const Vector& pos = self->getCurrentPosition();
	const Weapon* weap = armory.getCurrWeapon();
	if (enemy == nullptr) {
		inRange = true;
	} else if (weap != nullptr) {
		inRange = weap->isInRange(pos.DistTo(enemy->getCurrentPosition()));
	}
	if (blocker != nullptr) {
		if (BaseEntity(blocker).isDestroyedOrUsed()) {
			blackboard.setBlocker(nullptr);
			inRange = true;
		} else {
			extern IVEngineServer* engine;
			auto& players = blackboard.getPlayers();
			int entIndx = engine->IndexOfEdict(blocker);
			auto i = players.Find(entIndx);
			if (players.IsValidIndex(i) && players[i]->isDead()) {
				blackboard.setBlocker(nullptr);
				inRange = true;
			}
		}
		if (!inRange && weap != nullptr) {
			inRange = weap->isInRange(pos.DistTo(blocker->GetCollideable()->GetCollisionOrigin()));
		}
	} else if (enemy == nullptr) {
		inRange = true;
	}
	updateState(WorldProp::WEAPON_IN_RANGE, inRange);
	auto& weapons = armory.getWeapons();
	int currentWeap = armory.getCurrWeaponIdx();
	if (currentWeap > 0 && weapons.IsValidIndex(weapons.Find(currentWeap))) {
		Weapon *currWeap = weapons[weapons.Find(currentWeap)];
		updateState(WorldProp::WEAPON_LOADED,
				!currWeap->isClipEmpty());
		updateState(WorldProp::OUT_OF_AMMO,
				currWeap->isOutOfAmmo(self->getEdict()));
	}
	updateState(WorldProp::HEALTH_FULL, self->getHealth() >= self->getMaxHealth());
	bool hurt = states[states.Find(WorldProp::HURT)];
	updateState(WorldProp::HURT, false);
	updateState(WorldProp::IS_BLOCKED, blackboard.getBlocker() != nullptr);
	// reset planner if this is first time we see enemy.
	bool noEnemy = !enemySighted;
	enemySighted = enemy != nullptr;
	return update(blackboard) || (noEnemy && enemySighted) || hurt;
}
