#include "DODMGDeployerStartState.h"
#include "DODMGDeployer.h"
#include "DODMGDeployerProneState.h"
#include <player/Bot.h>
#include <player/Buttons.h>
#include <weapon/Weapon.h>
#include <in_buttons.h>

const int DODMGDeployerStartState::PRONE_TIMEOUT = 83,
DODMGDeployerStartState::DEPLOY_TIMEOUT = PRONE_TIMEOUT + 5;

void DODMGDeployerStartState::deploy(Bot *self) {
	if (context->getWeapon().isDeployed()) {
		context->setState(std::shared_ptr<DODMGDeployerState>(nullptr));
	} else if (wait++ <= 5) {
		self->getButtons().hold(IN_ATTACK2);
	} else {
		context->setState(std::make_shared<DODMGDeployerProneState>(context));
	}
}
