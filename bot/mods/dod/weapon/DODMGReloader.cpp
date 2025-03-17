#include "DODMGReloader.h"

#include <weapon/Deployer.h>
#include <weapon/Weapon.h>

bool DODMGReloader::execute(Bot *self) {
	return weapon.getDeployer()->execute(self)
			&& Reloader::execute(self);
}
