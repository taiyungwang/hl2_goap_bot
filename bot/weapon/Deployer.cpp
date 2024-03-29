#include "Deployer.h"

#include "Weapon.h"
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <in_buttons.h>

bool Deployer::execute(Blackboard& blackboard) {
	if (!weapon.isDeployed()) {
		blackboard.getButtons().tap(IN_ATTACK2);
	}
	return true;
}

void Deployer::undeploy(Blackboard& blackboard) {
	if (weapon.isDeployed()) {
		blackboard.getButtons().tap(IN_ATTACK2);
	}
}
