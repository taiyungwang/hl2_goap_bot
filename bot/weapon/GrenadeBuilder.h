#pragma once

#include "WeaponBuilder.h"

class GrenadeBuilder: public WeaponBuilder {
public:
	// TODO: Initial speed value taken from DOD, so may not apply for all mods.
	GrenadeBuilder(float zMultiplier, float initialSpeed = 500.0f) :
			zMultiplier(zMultiplier), initialSpeed(initialSpeed) {
	}

	virtual std::shared_ptr<Weapon> build(edict_t *weap) const;

protected:
	const float zMultiplier, initialSpeed;
};
