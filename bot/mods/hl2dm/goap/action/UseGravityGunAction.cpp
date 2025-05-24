#include "UseGravityGunAction.h"

#include <player/Bot.h>
#include <player/Vision.h>
#include <weapon/Weapon.h>
#include <weapon/WeaponFunction.h>

bool UseGravityGunAction::precondCheck() {
	return UseSpecificWeaponAction::precondCheck()
			&& Q_stristr(self->getBlocker()->GetClassName(), "physics")
					!= nullptr;
}

bool UseGravityGunAction::use() {
	edict_t* blocker = self->getBlocker();
	if (blocker == nullptr) {
		return true;
	}
	float dist = self->getViewTarget().DistTo(self->getCurrentPosition());
	if (dist > 130.0f) {
		self->setBlocker(nullptr);
		return true;
	}
	self->getCurrWeapon()->getPrimary()->attack(self->getButtons(),
				dist);
	return false;
}

bool UseGravityGunAction::canUse(int weapIdx) const {
	extern IVEngineServer* engine;
	return Q_strcmp(engine->PEntityOfEntIndex(weapIdx)->GetClassName(), "weapon_physcannon") == 0;
}
