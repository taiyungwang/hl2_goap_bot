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
	Arsenal& armory = blackboard.getSelf()->getArsenal();
	updateState(WorldProp::USING_BEST_WEAP,
			armory.getBestWeaponIdx() == armory.getCurrWeaponIdx());
	edict_t* blocker = blackboard.getBlocker();
	if (blocker != nullptr && BaseEntity(blocker).isDestroyedOrUsed()) {
		blocker = nullptr;
		blackboard.setBlocker(nullptr);
	}
	bool inRange = true;
	const Bot* self = blackboard.getSelf();
	const Weapon* weap = armory.getCurrWeapon();
	const Player* enemy = blackboard.getTargetedPlayer();
	if (weap != nullptr) {
		const Vector& pos = self->getCurrentPosition();
		if (enemy != nullptr) {
			inRange = weap->isInRange(pos.DistTo(enemy->getCurrentPosition()));
		} else if (blocker != nullptr) {
			extern IVEngineServer* engine;
			auto& players = Player::getPlayers();
			auto i = players.Find(engine->IndexOfEdict(blocker));
			if (players.IsValidIndex(i)) {
				if (players[i]->isDead()) {
					blackboard.setBlocker(nullptr);
				} else {
					blackboard.setViewTarget(players[i]->getEyesPos());
				}
			} else {
				blackboard.setViewTarget(blocker->GetCollideable()->GetCollisionOrigin());
			}
			if (blackboard.getBlocker() != nullptr) {
				inRange = weap->isInRange(pos.DistTo(blackboard.getViewTarget()));
			}
		}
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
