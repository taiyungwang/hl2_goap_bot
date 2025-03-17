#pragma once

#include "WeaponActor.h"

class Reloader: public WeaponActor {
public:
	Reloader(const Weapon& weap) : WeaponActor(weap) {
	}

	virtual ~Reloader() {
	}

	virtual bool execute(Bot *self);
};
