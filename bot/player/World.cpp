#include "World.h"

#include "Blackboard.h"
#include "Bot.h"
#include <voice/AreaClearVoiceMessage.h>
#include <weapon/Arsenal.h>
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
	states.Insert(WorldProp::HEARD_AREA_CLEAR, roundStarted);
	addStates();
}

bool World::think(Blackboard& blackboard) {
	bool& enemySighted = states[states.Find(WorldProp::ENEMY_SIGHTED)];
	Bot* self = blackboard.getSelf();
	updateState(WorldProp::MULTIPLE_ENEMY_SIGHTED,
			!self->getVision().getVisibleEnemies().empty());
	Arsenal& arsenal = blackboard.getSelf()->getArsenal();
	updateState(WorldProp::USING_BEST_WEAP,
			arsenal.getBestWeaponIdx() == arsenal.getCurrWeaponIdx());
	edict_t* blocker = blackboard.getBlocker();
	if (blocker != nullptr && BaseEntity(blocker).isDestroyedOrUsed()) {
		blocker = nullptr;
		blackboard.setBlocker(nullptr);
	}
	bool inRange = true;
	const Weapon* weap = arsenal.getCurrWeapon();
	const Player* enemy = self->getVision().getTargetedPlayer();
	if (weap != nullptr) {
		const Vector& pos = self->getCurrentPosition();
		if (enemy != nullptr) {
			inRange = weap->isInRange(pos.DistTo(enemy->getCurrentPosition()));
		} else if (blocker != nullptr) {
			extern IVEngineServer* engine;
			auto& players = Player::getPlayers();
			auto player = players.find(engine->IndexOfEdict(blocker));
			if (player != players.end()) {
				if (!self->isEnemy(*player->second)) {
					blackboard.setBlocker(nullptr);
				} else {
					self->setWantToListen(false);
					blackboard.setViewTarget(player->second->getEyesPos());
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
	Weapon* currWeap = arsenal.getCurrWeapon();
	if (currWeap != nullptr) {
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
	if (enemySighted && (enemy == nullptr || !self->canShoot(blackboard.getViewTarget(), enemy->getEdict()))
			&& !states[states.Find(WorldProp::HEARD_AREA_CLEAR)]) {
		updateState(WorldProp::HEARD_AREA_CLEAR, true);
	}
	enemySighted = enemy != nullptr;
	if (enemySighted) {
		updateState(WorldProp::HEARD_AREA_CLEAR, false);
	}
	return update(blackboard) || (noEnemy && enemySighted) || hurt;
}
