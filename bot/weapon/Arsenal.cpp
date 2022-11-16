#include "Arsenal.h"

#include "WeaponBuilder.h"
#include "Weapon.h"
#include "WeaponFunction.h"
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <util/SimpleException.h>
#include <util/BasePlayer.h>
#include <eiface.h>
#include <shareddefs.h>
#include <util/BaseCombatWeapon.h>

int Arsenal::getBestWeapon(Blackboard& blackboard, const WeaponFilter& ignore) const {
	int best = 0;
	Bot *self = blackboard.getSelf();
	auto* targetedPlayer = Player::getPlayer(self->getVision().getTargetedPlayer());
	edict_t* target = nullptr;
	if (targetedPlayer != nullptr) {
		target = targetedPlayer->getEdict();
	} else if (blackboard.getBlocker() != nullptr) {
		target = blackboard.getBlocker();
	}
	float targetDist = target == nullptr || target->IsFree() ? -1.0f:
			self->getViewTarget().DistTo(self->getEyesPos());
	visit([&best, targetDist, &blackboard=blackboard,
		   ignore, this](int i, const Weapon* weapon) mutable
			-> bool {
		edict_t* selfEdict = blackboard.getSelf()->getEdict();
		bool underWater = BasePlayer(selfEdict).isUnderWater();
		if ((!weapon->isUnderWater() && underWater)
				|| (targetDist < 0.0f && weapon->getPrimary()->isMelee() && !underWater)
				// TODO: assumes primary and secondary weapons have similar ranges.
				|| weapon->isGrenade() || weapon->isOutOfAmmo(selfEdict)
				|| ignore(weapon, blackboard, targetDist)
				|| (best > 0 && weapon->getDamage(selfEdict, targetDist)
								<= weapons.at(best)->getDamage(selfEdict, targetDist))) {
			return false;
		}
		best = i;
		return false;
	});
	return best;
}

void Arsenal::reset() {
	desiredWeapIdx = currWeapIdx = 0;
	weapons.clear();
}

void Arsenal::update(edict_t* self) {
	CBaseHandle* weapList = BaseEntity(self).getPtr<CBaseHandle>("m_hMyWeapons");
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
			weapons[entIdx] = builders.at(weapName).get()->build(weaponEnt);
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

