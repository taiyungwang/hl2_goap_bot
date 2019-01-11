#include "Armory.h"

#include "WeaponBuilder.h"
#include "Weapon.h"
#include "WeaponFunction.h"
#include "WeaponBuilderFactory.h"
#include <player/Blackboard.h>
#include <player/Player.h>
#include <util/EntityClassManager.h>
#include <util/EntityClass.h>
#include <util/SimpleException.h>
#include <util/BaseCombatWeapon.h>
#include <util/BasePlayer.h>
#include <eiface.h>
#include <shareddefs.h>

const char* Armory::getWeaponName(int key) {
	extern IVEngineServer* engine;
	edict_t* weap = engine->PEntityOfEntIndex(key);
	return weap == nullptr || weap->IsFree() ? nullptr: weap->GetClassName();
}

Armory::Armory()  {
	SetDefLessFunc(weapons);
	reset();
}

int Armory::getBestWeapon(Blackboard& blackboard, const WeaponFilter& ignore) const {
	auto best = weapons.InvalidIndex();
	auto* targetedPlayer = blackboard.getTargetedPlayer();
	edict_t* target = nullptr;
	if (targetedPlayer != nullptr) {
		target = targetedPlayer->getEdict();
	} else if (blackboard.getBlocker() != nullptr) {
		target = blackboard.getBlocker();
	}
	float targetDist = target == nullptr ? -1.0f:
			target->GetCollideable()->GetCollisionOrigin().DistTo(
					blackboard.getSelf()->getCurrentPosition());
	edict_t* self = blackboard.getSelf()->getEdict();
	bool underWater = BasePlayer(self).isUnderWater();
	FOR_EACH_MAP_FAST(weapons, i) {
		if ((!weapons[i]->isUnderWater() && underWater)
				|| (targetDist < 0.0f && weapons[i]->getPrimary()->isMelee()
						&& !underWater)
				// TODO: assumes primary and secondary weapons have similar ranges.
				|| weapons[i]->isGrenade() || weapons[i]->isOutOfAmmo(self)
				|| ignore(weapons[i], blackboard, targetDist)
				|| (weapons.IsValidIndex(best)
						&& weapons[i]->getDamage(self, targetDist)
								<= weapons[best]->getDamage(self, targetDist))) {
			continue;
		}
		best = i;
	}
	return weapons.IsValidIndex(best) ? weapons.Key(best): 0;
}

void Armory::reset() {
	currWeapIdx = bestWeapIdx = 0;
	weapons.Purge();
	FOR_EACH_MAP_FAST(weapons, i)
	{
		delete weapons[i];
		weapons[i] = nullptr;
	}
	weapons.RemoveAll();
}


void Armory::update(Blackboard& blackboard) {
	extern EntityClassManager* classManager;
	extern IVEngineServer* engine;
	edict_t* self = blackboard.getSelf()->getEdict();
	CBaseHandle* weapList =
			classManager->getClass("CBaseCombatCharacter")->getEntityVar(
					"m_hMyWeapons").getPtr<CBaseHandle>(self);
	auto current = weapons.InvalidIndex();
	for (int i = 0; i < MAX_WEAPONS; i++) {
		int entIdx = weapList[i].GetEntryIndex();
		edict_t* weaponEnt = engine->PEntityOfEntIndex(entIdx);
		if (weaponEnt == nullptr || weaponEnt->IsFree()) {
			continue;
		}
		int weapState = BaseCombatWeapon(weaponEnt).getWeaponState();
		if (weapState == WEAPON_NOT_CARRIED) {
			continue;
		}
		auto j = weapons.Find(entIdx);
		if (!weapons.IsValidIndex(j)) {
			WeaponBuilder* builder = factory.getInstance(
					weaponEnt->GetClassName());
			if (builder == nullptr) {
				throw SimpleException(
						CUtlString("Weapon is not registered: ")
								+ weaponEnt->GetClassName());
			}
			j = weapons.Insert(entIdx, builder->build(weaponEnt));
		}
		if (weapState == WEAPON_IS_ACTIVE) {
			current = j;
			currWeapIdx = weapons.Key(j);
		}
	}
	int best = getBestWeapon(blackboard,
			[] (const Weapon*, Blackboard&, float) {
		return false;
	});
	if (best != 0) {
		this->bestWeapIdx = best;
	}
}

Weapon* Armory::getWeapon(int key) const {
	auto index = weapons.Find(key);
	return weapons.IsValidIndex(index) ? weapons[index] : nullptr;
}

