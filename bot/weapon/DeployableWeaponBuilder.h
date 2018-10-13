#pragma once

#include "SemiAutoBuilder.h"

class EntityVar;

/**
 * Builds weapons that are zoomable, deployable, or have an alternate fire.
 */
class DeployableWeaponBuilder: public SemiAutoBuilder {
public:
	DeployableWeaponBuilder(float damage, float minRange, float maxRange,
			const char* className, const char* varName, float zoomDist = 0.0f);

	virtual Weapon* build(edict_t* weap);

private:
	EntityVar* deployableCheck;

	float zoomDist, minRange, maxRange;
};
