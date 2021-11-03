#include "DODUseRifleGrenadeAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
#include <player/Vision.h>
#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>
#include <weapon/WeaponFunction.h>

bool DODUseRifleGrenadeAction::precondCheck() {
	if (!DODUseSmokeGrenadeAction::precondCheck()) {
		return false;
	}
	Weapon *grenade = arsenal.getWeapon(weapIdx);
	target = nullptr;
	float minDist = grenade->getPrimary()->getRange()[0];
	auto enemies = blackboard.getSelf()->getVision().getVisibleEnemies();
	for(int i : enemies) {
		const Player* enemy = Player::getPlayer(i);
		if (enemy == nullptr) {
			continue;
		}
		Vector targetPos = enemy->getCurrentPosition();
		if (!grenade->isInRange(
				targetPos.DistTo(blackboard.getSelf()->getCurrentPosition()))) {
			continue;
		}
		float dist = 0;
		for(int j : enemies)
		{
			const Player* enemyOther = Player::getPlayer(j);
			if (enemyOther == nullptr || enemyOther == enemy) {
				continue;
			}
			dist += targetPos.DistTo(enemyOther->getCurrentPosition());
		}
		if (dist < minDist) {
			target = enemy;
			minDist = dist;
		}
	}
	return target != nullptr;
}

bool DODUseRifleGrenadeAction::canUse(const char* weaponName) const {
	return Q_strcmp(weaponName, "weapon_riflegren_us") == 0
			|| Q_strcmp(weaponName, "weapon_riflegren_ger") == 0;
}
