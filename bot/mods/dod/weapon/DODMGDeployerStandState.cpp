#include "DODMGDeployerStandState.h"
#include "DODMGDeployer.h"
#include <mods/dod/util/DodPlayer.h>
#include <player/Bot.h>
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <weapon/Weapon.h>
#include <in_buttons.h>

void DODMGDeployerStandState::deploy(Blackboard &blackboard) {
	if (!DodPlayer(blackboard.getSelf()->getEdict()).isProne()) {
		if (wait++ >= DEPLOY_TIMEOUT) {
			context->setState(std::shared_ptr<DODMGDeployerState>(nullptr));
		} else if (!context->getWeapon().isDeployed()) {
			blackboard.getButtons().hold(IN_ATTACK2);
			return;
		}
	}
	if (wait++ == 0) {
		blackboard.getButtons().tap(IN_ALT1);
	} else if (wait++ >= PRONE_TIMEOUT) {
		context->setState(std::shared_ptr<DODMGDeployerState>(nullptr));
	}
}
