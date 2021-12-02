#include "DODMGDeployer.h"
#include "DODMGDeployerStartState.h"
#include <weapon/Weapon.h>

bool DODMGDeployer::execute(Blackboard& blackboard) {
	if (!started && !weapon.isDeployed()) {
		started = true;
		state = std::make_shared<DODMGDeployerStartState>(this);
	}
	if (!state) {
		started = false;
		return true;
	}
	state->deploy(blackboard);
	return false;
}

void DODMGDeployer::undeploy(Blackboard& blackboard) {
	Deployer::undeploy(blackboard);
}

