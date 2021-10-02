#pragma once

#include "WeaponBuilder.h"

class UtilityToolBuilder: public WeaponBuilder {
public:
	UtilityToolBuilder(int maxRange = 0) :
			maxRange(maxRange) {
	}

	std::shared_ptr<Weapon> build(edict_t* weap) const;

private:
	int maxRange;
};
