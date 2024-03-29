#pragma once

#include "DODMGDeployer.h"
#include "DODMGReloader.h"
#include <weapon/DeployableWeaponBuilder.h>

class DODMGBuilder: public DeployableWeaponBuilder<DODMGReloader, DODMGDeployer> {
public:
	DODMGBuilder() :
			DeployableWeaponBuilder(0.8f, 500.0f, 3600.0f,
					"m_bDeployed", 500.0f) {
	}

	std::shared_ptr<Weapon> build(edict_t* weap) const;
};
