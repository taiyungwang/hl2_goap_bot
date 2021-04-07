#include "Deployer.h"

#include "Weapon.h"
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <in_buttons.h>

bool Deployer::execute(Blackboard& blackboard) {
	if (weapon.isDeployed()) {
		isDeploying = false;
		return true;
	}
	if (!isDeploying) {
		blackboard.getButtons().tap(IN_ATTACK2);
		isDeploying = true;
	}
	return false;
}

void Deployer::undeploy(Blackboard& blackboard) {
	isDeploying = false;
	if (weapon.isDeployed()) {
		blackboard.getButtons().tap(IN_ATTACK2);
	}
}
