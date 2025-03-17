#include "Deployer.h"

#include "Weapon.h"
#include <player/Bot.h>
#include <in_buttons.h>

bool Deployer::execute(Bot *self) {
	if (!weapon.isDeployed()) {
		self->getButtons().tap(IN_ATTACK2);
	}
	return true;
}

void Deployer::undeploy(Bot *self) {
	if (weapon.isDeployed()) {
		self->getButtons().tap(IN_ATTACK2);
	}
}
