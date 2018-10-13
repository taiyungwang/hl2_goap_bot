#include "DeployableWeaponBuilder.h"

#include "Weapon.h"
#include "WeaponFunction.h"
#include <util/EntityClassManager.h>
#include <util/EntityClass.h>
#include <util/EntityVar.h>

DeployableWeaponBuilder::DeployableWeaponBuilder(float damage, float minRange,
		float maxRange, const char* className, const char* varName,
		float zoomDist) :
		SemiAutoBuilder(damage), minRange(minRange), maxRange(maxRange), zoomDist(
				zoomDist) {
	extern EntityClassManager* classManager;
	deployableCheck = &classManager->getClass(className)->getEntityVar(varName);

}

Weapon* DeployableWeaponBuilder::build(edict_t* weap) {
	Weapon* weapon = SemiAutoBuilder::build(weap);
	weapon->setDeployable(deployableCheck, zoomDist);
	float* ranges = weapon->getPrimary()->getRange();
	ranges[0] = minRange;
	ranges[1] = maxRange;
	return weapon;
}
