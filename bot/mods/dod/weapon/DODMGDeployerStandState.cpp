#include "DODMGDeployerStandState.h"
#include "DODMGDeployer.h"
#include <mods/dod/util/DodPlayer.h>
#include <player/Bot.h>
#include <weapon/Weapon.h>
#include <in_buttons.h>

void DODMGDeployerStandState::deploy(Bot *self) {
	edict_t* selfEnt = self->getEdict();
	if (DodPlayer(selfEnt).isProne()) {
		if (wait++ == 0) {
			self->getButtons().hold(IN_ALT1);
		} else if (wait++ >= PRONE_TIMEOUT) {
			context->setState(std::shared_ptr<DODMGDeployerState>(nullptr));
		}
		return;
	}
	if (context->getWeapon().isDeployed() || wait++ >= DEPLOY_TIMEOUT) {
		context->setState(std::shared_ptr<DODMGDeployerState>(nullptr));
		return;
	}
	self->getButtons().hold(IN_ATTACK2);
}
