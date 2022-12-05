#pragma once

#include "WeaponBuilder.h"

class UtilityToolBuilder: public WeaponBuilder {
public:
	UtilityToolBuilder(float maxRange = 0.0f) :
			maxRange(maxRange) {
	}

	std::shared_ptr<Weapon> build(edict_t* weap) const;

private:
	float maxRange;
};
