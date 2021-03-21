#pragma once

#include "WeaponActor.h"

class Deployer: public WeaponActor {
public:
	Deployer(const Weapon& weapon): WeaponActor(weapon) {
	}

	virtual ~Deployer() {
	}

	virtual bool execute(Blackboard& blackboard);

	virtual void undeploy(Blackboard& blackboard);
};
