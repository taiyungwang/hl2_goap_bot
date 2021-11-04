#include "Arsenal.h"

#include "WeaponBuilder.h"
#include "Weapon.h"
#include "WeaponFunction.h"
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <util/EntityClassManager.h>
#include <util/EntityClass.h>
#include <util/SimpleException.h>
#include <util/BaseCombatWeapon.h>
#include <util/BasePlayer.h>
#include <eiface.h>
#include <shareddefs.h>

int Arsenal::getBestWeapon(Blackboard& blackboard, const WeaponFilter& ignore) const {
	int best = 0;
	auto* targetedPlayer = Player::getPlayer(blackboard.getSelf()->getVision().getTargetedPlayer());
	edict_t* target = nullptr;
	if (targetedPlayer != nullptr) {
		target = targetedPlayer->getEdict();
	} else if (blackboard.getBlocker() != nullptr) {
		target = blackboard.getBlocker();
	}
	float targetDist = target == nullptr ? -1.0f:
			target->GetCollideable()->GetCollisionOrigin().DistTo(
					blackboard.getSelf()->getCurrentPosition());
	visit([&best, targetDist, &blackboard=blackboard,
		   ignore, this](int i, const Weapon* weapon) mutable
			-> bool {
		edict_t* self = blackboard.getSelf()->getEdict();
		bool underWater = BasePlayer(self).isUnderWater();
		if ((!weapon->isUnderWater() && underWater)
				|| (targetDist < 0.0f && weapon->getPrimary()->isMelee() && !underWater)
				// TODO: assumes primary and secondary weapons have similar ranges.
				|| weapon->isGrenade() || weapon->isOutOfAmmo(self)
				|| ignore(weapon, blackboard, targetDist)
				|| (best > 0 && weapon->getDamage(self, targetDist)
								<= weapons.at(best)->getDamage(self, targetDist))) {
			return false;
		}
		best = i;
		return false;
	});
	return best;
}

void Arsenal::reset() {
	currWeapIdx = bestWeapIdx = 0;
	weapons.clear();
}

void Arsenal::update(edict_t* self) {
	extern EntityClassManager* classManager;
	CBaseHandle* weapList =
			classManager->getClass("CBaseCombatCharacter")->getEntityVar(
					"m_hMyWeapons").getPtr<CBaseHandle>(self);
	for (int i = 0; i < MAX_WEAPONS; i++) {
		int entIdx = weapList[i].GetEntryIndex();
		extern IVEngineServer* engine;
		edict_t* weaponEnt = engine->PEntityOfEntIndex(entIdx);
		if (weaponEnt == nullptr || weaponEnt->IsFree()) {
			continue;
		}
		int weapState = BaseCombatWeapon(weaponEnt).getWeaponState();
		if (weapState == WEAPON_NOT_CARRIED) {
			continue;
		}
		if (weapons.find(entIdx) == weapons.end()) {
			const char* weapName = weaponEnt->GetClassName();
			if (builders.find(weapName) == builders.end()) {
				Warning("Weapon is not registered: %s.\n", weapName);
				continue;
			}
			const WeaponBuilder* builder = builders.at(weapName).get();
			weapons[entIdx] = builder->build(weaponEnt);
		}
		if (weapState == WEAPON_IS_ACTIVE) {
			currWeapIdx = entIdx;
		}
	}
}

int Arsenal::getWeaponIdByName(const char* name) const {
	int id = 0;
	visit([&id, name] (int i, const Weapon* weapon) mutable -> bool {
		extern IVEngineServer* engine;
		edict_t* weap = engine->PEntityOfEntIndex(i);
		if (weap != nullptr && !weap->IsFree()
				&& std::string(name) == weap->GetClassName()) {
			id = i;
			return true;
		}
		return false;
	});
	return id;
}

