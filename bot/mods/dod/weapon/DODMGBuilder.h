#pragma once

#include "DODMGDeployer.h"
#include "DODMGReloader.h"
#include <weapon/DeployableWeaponBuilder.h>

class DODMGBuilder: public DeployableWeaponBuilder<DODMGReloader, DODMGDeployer> {
public:
	DODMGBuilder() :
			DeployableWeaponBuilder(0.8f, 0.0f, 3600.0f, "CDODBipodWeapon",
					"m_bDeployed", 200.0f) {
	}

	std::shared_ptr<Weapon> build(edict_t* weap) const;
};
