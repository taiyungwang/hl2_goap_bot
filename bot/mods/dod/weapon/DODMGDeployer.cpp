#include "DODMGDeployer.h"
#include "DODMGDeployerStartState.h"
#include <weapon/Weapon.h>

bool DODMGDeployer::execute(Bot *self) {
	if (!started && !weapon.isDeployed()) {
		started = true;
		state = std::make_shared<DODMGDeployerStartState>(this);
	}
	if (!state) {
		started = false;
		return true;
	}
	state->deploy(self);
	return false;
}

void DODMGDeployer::undeploy(Bot *self) {
	Deployer::undeploy(self);
}

