#include "DODMGReloader.h"

#include <weapon/Deployer.h>
#include <weapon/Weapon.h>

bool DODMGReloader::execute(Blackboard& blackboard) {
	if (!weapon.getDeployer()->execute(blackboard)) {
		return false;
	}
	return Reloader::execute(blackboard);
}
