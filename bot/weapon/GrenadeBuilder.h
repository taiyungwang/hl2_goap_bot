#pragma once

#include "WeaponBuilder.h"

class GrenadeBuilder: public WeaponBuilder {
public:
	GrenadeBuilder(float initialSpeed = 1000.0f) :
			initialSpeed(initialSpeed) {
	}

	virtual std::shared_ptr<Weapon> build(edict_t *weap) const;

protected:
	const float initialSpeed;
};
