#pragma once

#include "ReloadableWeaponBuilder.h"
#include "Weapon.h"
#include "WeaponFunction.h"
#include "Deployer.h"
#include <util/EntityClassManager.h>
#include <util/EntityVar.h>
#include <util/EntityClass.h>

/**
 * Builds weapons that are zoomable, deployable, or have an alternate fire.
 */
template<typename T, typename U = Deployer>
class DeployableWeaponBuilder: public ReloadableWeaponBuilder<T> {
public:
	DeployableWeaponBuilder(float damage, float minRange, float maxRange,
			const char* className, const char* varName, float zoomDist = 0.0f) :
			damage(damage), minRange(minRange), maxRange(maxRange),
			zoomDist(zoomDist) {
		extern EntityClassManager* classManager;
		deployableCheck = &classManager->getClass(className)->getEntityVar(
				varName);
	}

	virtual ~DeployableWeaponBuilder() {
	}

	virtual Weapon* build(edict_t* weap) {
		Weapon* weapon = ReloadableWeaponBuilder<T>::build(weap);
		weapon->setPrimary(new WeaponFunction(damage));
		weapon->getPrimary()->getRange()[1] = 1000.0f;
		weapon->setDeployable(deployableCheck, zoomDist);
		weapon->setDeployer(new U(*weapon));
		float* ranges = weapon->getPrimary()->getRange();
		ranges[0] = minRange;
		ranges[1] = maxRange;
		return weapon;
	}

private:
	EntityVar* deployableCheck;

	float damage, zoomDist, minRange, maxRange;
};
