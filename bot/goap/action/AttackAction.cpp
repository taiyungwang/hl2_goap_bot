#include "AttackAction.h"

#include <player/Blackboard.h>
#include <player/Player.h>
#include <player/Vision.h>
#include <weapon/WeaponFunction.h>
#include <weapon/Weapon.h>
#include <navmesh/nav.h>
#include <util/UtilTrace.h>
#include <convar.h>
#include <edict.h>

AttackAction::AttackAction(Blackboard& blackboard) :
		DestroyObjectAction(blackboard) {
	effects = {WorldProp::ENEMY_SIGHTED, false};
}

bool AttackAction::precondCheck() {
	return blackboard.getTargetedPlayer() != nullptr;
}

bool AttackAction::postCondCheck() {
	return blackboard.getTargetedPlayer() == nullptr
			|| blackboard.getTargetedPlayer()->isDead();
}

bool AttackAction::isVisible(const Vector& end, edict_t* target) const {
	return UTIL_IsVisible(end, blackboard, target);
}

static ConVar mybot_aim_offset("mybot_aim_offset", "5");

void AttackAction::getAdjustedTargetPos(Vector& targetLoc,
		WeaponFunction* weapFunc) const {
	auto self = blackboard.getSelf();
	Vector eyes = self->getEyesPos();
	auto targeted = blackboard.getTargetedPlayer();
	if (!weapFunc->isExplosive()) {
		edict_t *selfEnt = self->getEdict(), *targetEnt = targeted->getEdict();
		Vector head = targeted->getEyesPos();
		head.z -= 8.0f;
		Vector chest = head;
		chest.z -= 10.0f;
		if (UTIL_IsTargetHit(eyes, chest, selfEnt, targetEnt)) {
			targetLoc = chest;
		}
		Vector aim = (head - eyes).Normalized();
		Vector headPos[] = { head, Vector(-aim.y, aim.x, aim.z)
				* mybot_aim_offset.GetFloat() + head, Vector(aim.y, -aim.x,
				aim.z) * mybot_aim_offset.GetFloat() + head };
		float dist = eyes.DistTo(head);
		for (int i = 0; i < 3; i++) {
			if ((i == 0 && weapFunc->isMelee())
					|| UTIL_IsTargetHit(eyes, head, selfEnt, targetEnt)) {
				targetLoc = headPos[i];
				break;
			}
		}
	}
	targetLoc = weapFunc->getAim(targetLoc, eyes);
}

edict_t* AttackAction::getTargetedEdict() const {
	return blackboard.getTargetedPlayer()->getEdict();
}
