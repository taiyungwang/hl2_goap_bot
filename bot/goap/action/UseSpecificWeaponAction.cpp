#include "UseSpecificWeaponAction.h"

#include <player/Blackboard.h>
#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>

UseSpecificWeaponAction::UseSpecificWeaponAction(Blackboard& blackboard) :
		WeaponAction(blackboard) {
	precond[WorldProp::USING_DESIRED_WEAPON] = true;
}

bool UseSpecificWeaponAction::precondCheck() {
	weapIdx = 0;
	arsenal.visit([this](int i, const Weapon* weapon) mutable -> bool {
		extern IVEngineServer* engine;
		edict_t* ent = engine->PEntityOfEntIndex(i);
		if (ent != nullptr && !ent->IsFree()
				&& this->canUse(ent->GetClassName())) {
			weapIdx = i;
			arsenal.setDesiredWeaponIdx(weapIdx);
			return true;
		}
		return false;
	});
	return weapIdx > 0;
}
