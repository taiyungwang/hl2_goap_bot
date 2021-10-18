#include "DODUseFragGrenadeAction.h"

#include <mods/dod/voice/DODVoiceMessage.h>
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <player/Vision.h>
#include <voice/VoiceMessageSender.h>
#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>
#include <weapon/WeaponFunction.h>

bool DODUseFragGrenadeAction::precondCheck() {
	bool success = DODUseRifleGrenadeAction::precondCheck();
	if (success) {
		primeDuration = 0;
	}
	return success;
}

bool DODUseFragGrenadeAction::execute() {
	Weapon* weapon = arsenal.getWeapon(weapIdx);
	Bot* self = blackboard.getSelf();
	if (!DODUseSmokeGrenadeAction::precondCheck() || primeDuration++ >= 300) {
		self->getVoiceMessageSender().sendMessage(std::make_shared<DODVoiceMessage::FireInTheHole>(self->getEdict()));
		return true;
	}
	float dist = blackboard.getTargetEntDistance();
	WeaponFunction* grenade = weapon->chooseWeaponFunc(self->getEdict(),
			dist);
	self->setWantToListen(false);
	blackboard.setViewTarget(grenade->getAim(target->getCurrentPosition(),
			self->getEyesPos()));
	grenade->attack(blackboard.getButtons(), dist);
	return false;
}

bool DODUseFragGrenadeAction::canUse(const char* weaponName) const {
	return Q_strcmp(weaponName, "weapon_frag_us") == 0
			|| Q_strcmp(weaponName, "weapon_frag_ger") == 0;
}
