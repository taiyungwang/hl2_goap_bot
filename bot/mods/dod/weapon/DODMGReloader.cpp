#include "DODMGReloader.h"

#include <weapon/Deployer.h>
#include <weapon/Weapon.h>

bool DODMGReloader::execute(Blackboard &blackboard) {
	return weapon.getDeployer()->execute(blackboard)
			&& Reloader::execute(blackboard);
}
