#include "DODUseRifleGrenadeAction.h"

#include <player/Blackboard.h>
#include <player/Player.h>
#include <player/Vision.h>
#include <weapon/Weapon.h>
#include <weapon/WeaponFunction.h>

bool DODUseRifleGrenadeAction::precondCheck() {
	if (!DODUseSmokeGrenadeAction::precondCheck()) {
		return false;
	}
	Weapon *grenade = armory.getWeapon(weapIdx);
	target = nullptr;
	auto& enemies = blackboard.getVisibleEnemies();
	float minDist = grenade->getPrimary()->getRange()[0];
	FOR_EACH_VEC(enemies, i)
	{
		Vector targetPos = enemies[i]->getCurrentPosition();
		if (!grenade->isInRange(
				targetPos.DistTo(blackboard.getSelf()->getCurrentPosition()))) {
			continue;
		}
		FOR_EACH_VEC(enemies, j)
		{
			if (enemies[i] == enemies[j]) {
				continue;
			}
			float dist = targetPos.DistTo(enemies[j]->getCurrentPosition());
			if (dist < minDist) {
				target = enemies[i];
				minDist = dist;
			}
		}
	}
	return target != nullptr;
}

bool DODUseRifleGrenadeAction::canUse(const char* weaponName) const {
	return Q_strcmp(weaponName, "weapon_riflegren_us") == 0
			|| Q_strcmp(weaponName, "weapon_riflegren_ger") == 0;
}
