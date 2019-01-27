#pragma once

#include "SemiAutoBuilder.h"
#include "Weapon.h"
#include "WeaponFunction.h"
#include "Deployer.h"
#include <util/EntityClassManager.h>
#include <util/EntityVar.h>
#include <util/EntityClass.h>

/**
 * Builds weapons that are zoomable, deployable, or have an alternate fire.
 */
template<typename T = Deployer>
class DeployableWeaponBuilder: public SemiAutoBuilder {
public:
	DeployableWeaponBuilder(float damage, float minRange, float maxRange,
			const char* className, const char* varName, float zoomDist = 0.0f) :
			SemiAutoBuilder(damage), minRange(minRange), maxRange(maxRange),
			zoomDist(zoomDist) {
		extern EntityClassManager* classManager;
		deployableCheck = &classManager->getClass(className)->getEntityVar(
				varName);
	}

	virtual Weapon* build(edict_t* weap) {
		Weapon* weapon = SemiAutoBuilder::build(weap);
		weapon->setDeployable(deployableCheck, zoomDist);
		weapon->setDeployer(new T(*weapon));
		float* ranges = weapon->getPrimary()->getRange();
		ranges[0] = minRange;
		ranges[1] = maxRange;
		return weapon;
	}

private:
	EntityVar* deployableCheck;

	float zoomDist, minRange, maxRange;
};
