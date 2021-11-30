#include "DODMGDeployerStandState.h"
#include "DODMGDeployer.h"
#include "DODMGDeployerDeployState.h"
#include <mods/dod/util/DodPlayer.h>
#include <player/Bot.h>
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <in_buttons.h>

void DODMGDeployerStandState::deploy(Blackboard &blackboard) {
	if (frames++ > 60
			&& blackboard.getButtons().tap(IN_ATTACK2)) {
		context->setState(std::make_shared<DODMGDeployerDeployState>(context));
	}
}
