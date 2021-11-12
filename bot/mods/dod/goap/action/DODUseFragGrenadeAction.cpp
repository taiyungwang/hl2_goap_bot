#include "DODUseFragGrenadeAction.h"

#include <mods/dod/voice/DODVoiceMessage.h>
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <player/Vision.h>
#include <voice/VoiceMessageSender.h>
#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>
#include <util/EntityClassManager.h>
#include <util/EntityClass.h>
#include <util/EntityVar.h>
#include <in_buttons.h>

bool DODUseFragGrenadeAction::execute() {
	Bot* self = blackboard.getSelf();
	self->setViewTarget(target);
	if (self->getVision().getVisibleEnemies().size() < 2 || primeDuration++ >= 300) {
		extern IVEngineServer *engine;
		edict_t *weap = engine->PEntityOfEntIndex(self->getArsenal().getCurrWeaponIdx());
		extern EntityClassManager* classManager;
		if (classManager->getClass("CWeaponDODBaseGrenade")->getEntityVar("m_bPinPulled").get<bool>(weap)) {
			self->getVoiceMessageSender().sendMessage(std::make_shared<DODVoiceMessage::FireInTheHole>(self->getEdict()));
		}
		return true;
	}
	self->setWantToListen(false);
	blackboard.getButtons().hold(IN_ATTACK);
	return false;
}

bool DODUseFragGrenadeAction::canUse(const char* weaponName) const {
	return Q_strcmp(weaponName, "weapon_frag_us") == 0
			|| Q_strcmp(weaponName, "weapon_frag_ger") == 0;
}
