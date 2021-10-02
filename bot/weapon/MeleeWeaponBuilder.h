#pragma once

#include "WeaponBuilder.h"

class MeleeWeaponBuilder: public WeaponBuilder {
public:
	std::shared_ptr<Weapon> build(edict_t* weap) const;
};
