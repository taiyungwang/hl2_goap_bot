#pragma once

#include <weapon/DeployableWeaponBuilder.h>
#include <weapon/Reloader.h>

class DODAssaultRifleBuilder: public DeployableWeaponBuilder<Reloader> {
public:
	DODAssaultRifleBuilder() :
			DeployableWeaponBuilder<Reloader>(0.8f, 100.0f, 1600.0f,
					"CDODFireSelectWeapon", "m_bSemiAuto", 1000.0f) {
	}

	Weapon* build(edict_t* weap);
};
