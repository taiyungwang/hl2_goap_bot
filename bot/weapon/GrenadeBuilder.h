#pragma once

#include "WeaponBuilder.h"

class GrenadeBuilder: public WeaponBuilder {
public:
	GrenadeBuilder(float zMultiplier) :
			zMultiplier(zMultiplier) {
	}

	virtual std::shared_ptr<Weapon> build(edict_t *weap) const;

protected:
	const float zMultiplier;
};
