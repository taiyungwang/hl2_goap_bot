#pragma once

#include "ReloadableWeaponBuilder.h"

class RPGBuilder: public ReloadableWeaponBuilder<> {
public:
	std::shared_ptr<Weapon> build(edict_t* weap) const;
};
