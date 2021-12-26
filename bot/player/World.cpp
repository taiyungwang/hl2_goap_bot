#include "World.h"

#include "Blackboard.h"
#include "Bot.h"
#include <voice/AreaClearVoiceMessage.h>
#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>
#include <util/BaseEntity.h>
#include <eiface.h>

void World::reset() {
	states.clear();
	states[WorldProp::USING_DESIRED_WEAPON] = false;
	states[WorldProp::ENEMY_SIGHTED] = false;
	states[WorldProp::MULTIPLE_ENEMY_SIGHTED] = false;
	states[WorldProp::HURT] = false ;
	states[WorldProp::HEALTH_FULL] = true ;
	states[WorldProp::IS_BLOCKED] = false;
	states[WorldProp::USING_BEST_WEAP] = false;
	states[WorldProp::WEAPON_LOADED] = true;
	states[WorldProp::OUT_OF_AMMO] = false;
	states[WorldProp::WEAPON_IN_RANGE] = false;
	states[WorldProp::ROUND_STARTED] = roundStarted;
	states[WorldProp::HEARD_AREA_CLEAR] = false;
	states[WorldProp::EXPLOSIVE_NEAR] = false;
	addStates();
}

bool World::think(Blackboard& blackboard) {
	bool& enemySighted = states[WorldProp::ENEMY_SIGHTED];
	updateState(WorldProp::HEARD_AREA_CLEAR, false);
	updateState(WorldProp::EXPLOSIVE_NEAR, false);
	Bot* self = blackboard.getSelf();
	updateState(WorldProp::MULTIPLE_ENEMY_SIGHTED,
			self->getVision().getVisibleEnemies().size() > 1);
	Arsenal& arsenal = blackboard.getSelf()->getArsenal();
	edict_t* blocker = blackboard.getBlocker();
	if (blocker != nullptr && (BaseEntity(blocker).isDestroyedOrUsed()
			|| (self->getFacing().Dot(blocker->GetCollideable()->GetCollisionOrigin()
					- self->getCurrentPosition()) > 0.0f
					&& !self->canSee(blocker)))) {
		blocker = nullptr;
		blackboard.setBlocker(nullptr);
	}
	bool inRange = true;
	const Weapon* weap = arsenal.getCurrWeapon();
	const Player* enemy = Player::getPlayer(self->getVision().getTargetedPlayer());
	if (weap != nullptr) {
		const Vector& pos = self->getEyesPos();
		if (enemy != nullptr) {
			inRange = weap->isInRange(enemy->getEyesPos().DistTo(pos));
		} else if (blocker != nullptr) {
			extern IVEngineServer* engine;
			auto& players = Player::getPlayers();
			auto player = players.find(engine->IndexOfEdict(blocker));
			if (player != players.end()) {
				if (!self->isEnemy(*player->second)) {
					blackboard.setBlocker(nullptr);
				} else {
					self->setWantToListen(false);
					self->setViewTarget(player->second->getEyesPos());
				}
			} else {
				Vector target = blocker->GetCollideable()->GetCollisionOrigin();
				target.z += blocker->GetCollideable()->OBBMaxs().z / 2.0f;
				self->setViewTarget(target);
			}
			if (blackboard.getBlocker() != nullptr) {
				inRange = weap->isInRange(pos.DistTo(self->getViewTarget()));
			}
		}
		updateState(WorldProp::WEAPON_IN_RANGE, inRange);
		updateState(WorldProp::WEAPON_LOADED, !weap->isClipEmpty());
		updateState(WorldProp::OUT_OF_AMMO, weap->isOutOfAmmo(self->getEdict()));
	}
	updateState(WorldProp::HEALTH_FULL, self->getHealth() >= self->getMaxHealth());
	bool hurt = states[WorldProp::HURT];
	updateState(WorldProp::HURT, false);
	updateState(WorldProp::IS_BLOCKED, blackboard.getBlocker() != nullptr);
	// reset planner if this is first time we see enemy.
	bool noEnemy = !enemySighted;
	if (enemySighted && enemy == nullptr
			&& !states[WorldProp::HEARD_AREA_CLEAR]) {
		updateState(WorldProp::HEARD_AREA_CLEAR, true);
	}
	enemySighted = enemy != nullptr;
	return update(blackboard) || (noEnemy && enemySighted) || hurt;
}
