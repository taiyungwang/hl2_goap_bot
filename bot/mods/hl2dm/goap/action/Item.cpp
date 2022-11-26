#include "Item.h"
#include <util/BaseCombatWeapon.h>
#include <edict.h>
#include <string>

bool Item::isAvailable() const {
	return !BaseEntity(ent).isDestroyedOrUsed()
			&& (std::string(ent->GetClassName()).find("weapon_") != 0
			|| BaseCombatWeapon(ent).getOwner() == nullptr);
}

bool Charger::isAvailable() const {
	return BaseEntity(ent).get<float>("m_flCycle", 1.0f) < 1.0f;
}
