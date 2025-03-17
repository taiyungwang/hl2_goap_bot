#pragma once

#include <weapon/Reloader.h>

class DODMGReloader: public Reloader {
public:
	DODMGReloader(const Weapon& weap): Reloader(weap) {
	}

	bool execute(Bot *self);
};
