#pragma once

#include "WeaponActor.h"

class Deployer: public WeaponActor {
public:
	Deployer(const Weapon& weapon): WeaponActor(weapon) {
	}

	virtual ~Deployer() {
	}

	virtual bool execute(Bot *self);

	virtual void undeploy(Bot *self);
};
