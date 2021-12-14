#include "Item.h"
#include <util/BaseCombatWeapon.h>
#include <util/EntityClassManager.h>
#include <edict.h>

bool Item::isAvailable() const {
	return !BaseEntity(ent).isDestroyedOrUsed()
			&& (std::string(ent->GetClassName()).find("weapon_") != 0
			|| BaseCombatWeapon(ent).getOwner() == nullptr);
}

bool Charger::isAvailable() const {
	extern EntityClassManager *classManager;
	return classManager->getClass("CBaseAnimating")->getEntityVar("m_flCycle")
			.get<float>(ent) == 1.0f;
}
