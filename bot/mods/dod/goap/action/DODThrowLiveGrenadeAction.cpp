#include "DODThrowLiveGrenadeAction.h"

#include <mods/dod/weapon/DODLiveGrenadeBuilder.h>
#include <player/Blackboard.h>
#include <player/Bot.h>

const Player* DODThrowLiveGrenadeAction::chooseTarget() const {
	float closest = INFINITY;
	auto self = blackboard.getSelf();
	auto vision = self->getVision();
	const Player *target = nullptr;
	for (auto i: vision.getVisibleEnemies()) {
		auto enemy = Player::getPlayer(i);
		float dist = self->getCurrentPosition().DistTo(enemy->getCurrentPosition());
		if (dist < closest) {
			target = enemy;
			closest = dist;
		}
	}
	return target;
}

bool DODThrowLiveGrenadeAction::canUse(const char *weaponName) const {
	return DODLiveGrenadeBuilder::NAMES.find(weaponName)
			!= DODLiveGrenadeBuilder::NAMES.end();
}
