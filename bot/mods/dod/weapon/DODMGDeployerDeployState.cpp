#include "DODMGDeployerDeployState.h"

#include "DODMGDeployer.h"
#include <weapon/Weapon.h>

void DODMGDeployerDeployState::deploy(Blackboard &blackboard) {
	if (context->getWeapon().isDeployed() || frames++ >= 5) {
		context->setState(std::shared_ptr<DODMGDeployerState>(nullptr));
	}
}
