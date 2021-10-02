#pragma once

#include "ReloadableWeaponBuilder.h"

class SemiAutoBuilder: public ReloadableWeaponBuilder<> {
public:
	SemiAutoBuilder(float damage1) {
		this->damage1 = damage1;
	}

	virtual std::shared_ptr<Weapon> build(edict_t* weap) const;
};
