#pragma once

#include "DODMGDeployer.h"
#include <weapon/DeployableWeaponBuilder.h>

class DODMGBuilder: public DeployableWeaponBuilder<DODMGDeployer> {
public:
	DODMGBuilder() :
			DeployableWeaponBuilder(0.8f, 300.0f, 3600.0f, "CDODBipodWeapon",
					"m_bDeployed", -1.0f) {
	}

	Weapon* build(edict_t* weap);
};
