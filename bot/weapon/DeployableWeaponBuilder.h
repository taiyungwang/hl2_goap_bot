#pragma once

#include "ReloadableWeaponBuilder.h"
#include "Weapon.h"
#include "WeaponFunction.h"
#include "Deployer.h"
#include <string>

/**
 * Builds weapons that are zoomable, deployable, or have an alternate fire.
 */
template<typename T, typename U = Deployer>
class DeployableWeaponBuilder: public ReloadableWeaponBuilder<T> {
public:
	DeployableWeaponBuilder(float damage, float minRange, float maxRange,
			const char* varName, float zoomDist = 0.0f) :
			damage(damage), minRange(minRange), maxRange(maxRange), deployableCheck(varName),
			zoomDist(zoomDist) {
	}

	virtual ~DeployableWeaponBuilder() {
	}

	virtual std::shared_ptr<Weapon> build(edict_t* weap) const {
		auto weapon = ReloadableWeaponBuilder<T>::build(weap);
		weapon->setPrimary(std::make_shared<WeaponFunction>(damage));
		weapon->getPrimary()->getRange()[1] = 1000.0f;
		weapon->setDeployable(deployableCheck.c_str(), zoomDist);
		weapon->template setDeployer<U>();
		float* ranges = weapon->getPrimary()->getRange();
		ranges[0] = minRange;
		ranges[1] = maxRange;
		return weapon;
	}

private:
	const std::string deployableCheck;

	std::shared_ptr<U> deplolyer;

	float damage, zoomDist, minRange, maxRange;
};
