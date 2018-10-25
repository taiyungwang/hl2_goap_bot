#pragma once

#include "WeaponBuilder.h"

class UtilityToolBuilder: public WeaponBuilder {
public:
	UtilityToolBuilder(int maxRange = 0) :
			maxRange(maxRange) {
	}

	Weapon* build(edict_t* weap);

private:
	int maxRange;
};
