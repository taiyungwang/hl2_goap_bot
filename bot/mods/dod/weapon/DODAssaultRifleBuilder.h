#pragma once

#include <weapon/DeployableWeaponBuilder.h>
#include <weapon/Reloader.h>

class DODAssaultRifleBuilder: public DeployableWeaponBuilder<Reloader> {
public:
	DODAssaultRifleBuilder() :
			DeployableWeaponBuilder<Reloader>(0.8f, Weapon::MELEE_RANGE, 1600.0f,
					"CDODFireSelectWeapon", "m_bSemiAuto", 1000.0f) {
	}

	std::shared_ptr<Weapon> build(edict_t* weap) const override;
};
