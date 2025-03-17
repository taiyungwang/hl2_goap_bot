#include "KillAction.h"

#include <player/Bot.h>
#include <weapon/Weapon.h>
#include <limits>

static ConVar mybotAttackDelay("mybot_attack_delay", "60");

static ConVar mybotAimVar("mybot_aim_variance", "1.0f", 0,
		"range of randomness for a bot's aim");

KillAction::KillAction(Bot *self) :
		AttackAction(self) {
	effects = { WorldProp::ENEMY_SIGHTED, false };
}

bool KillAction::precondCheck() {
	framesToWait = mybotAttackDelay.GetInt();
	adjustAim = true;
	dur = UINT_MAX;
	target = self->getVision().getTargetedPlayer();
	if (target > 0) {
		self->setAimOffset(mybotAimVar.GetFloat());
		return true;
	}
	return false;
}

bool KillAction::execute()  {
	int visionTarget = self->getVision().getTargetedPlayer();
	if (target != visionTarget) {
		target = visionTarget;
		framesToWait = mybotAttackDelay.GetInt();
	}
	const Player *player = Player::getPlayer(target);
	auto weapon = self->getCurrWeapon();
	return ((weapon && weapon->getDeployer() != nullptr
			&& player != nullptr && weapon->getMinDeployRange()
			< player->getCurrentPosition().DistTo(self->getCurrentPosition())
			&& !weapon->isDeployed())
			|| --framesToWait <= 0) && AttackAction::execute();
}

bool KillAction::targetDestroyed() const {
	const Player* targetedPlayer = Player::getPlayer(target);
	return targetedPlayer == nullptr || !targetedPlayer->isInGame();
}

edict_t* KillAction::getTargetedEdict() const {
	const Player* player = Player::getPlayer(target);
	return player == nullptr || !player->isInGame() ? nullptr : player->getEdict();
}
