#include "Deployer.h"

#include "Weapon.h"
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <in_buttons.h>

bool Deployer::deploy(Blackboard& blackboard) {
	if (!weapon.isDeployed()) {
		blackboard.getButtons().tap(IN_ATTACK2);
	}
	return true;
}
