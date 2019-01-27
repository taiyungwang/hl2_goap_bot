#pragma once

#include <weapon/DeployableWeaponBuilder.h>

class DODAssaultRifleBuilder: public DeployableWeaponBuilder<> {
public:
	DODAssaultRifleBuilder() :
			DeployableWeaponBuilder(0.8f, 100.0f, 1600.0f,
					"CDODFireSelectWeapon", "m_bSemiAuto", 1000.0f) {
	}

	Weapon* build(edict_t* weap);
};
