#include "DeployMGAction.h"

#include <mods/dod/util/DodPlayer.h>
#include <player/Blackboard.h>
#include <player/Player.h>
#include <player/Vision.h>
#include <weapon/Weapon.h>
#include <navmesh/nav.h>
#include <util/BasePlayer.h>
#include <util/UtilTrace.h>
#include <in_buttons.h>

DeployMGAction::DeployMGAction(Blackboard& blackboard) :
		WeaponAction(blackboard) {
	effects = {WorldProp::NEED_TO_DEPLOY_WEAPON, false};
}

bool DeployMGAction::precondCheck() {
	if (!armory.getCurrWeapon()->isDeployable()) {
		return false;
	}
	const Player* player = blackboard.getTargetedPlayer();
	Vector target;
	if (player != nullptr) {
		target = player->getEyesPos();
	} else {
		edict_t* blocker = blackboard.getBlocker();
		if (blocker == nullptr) {
			return false;
		}
		target = blocker->GetCollideable()->GetCollisionOrigin();
	}
	const Player* self = blackboard.getSelf();
	Vector pos = self->getEyesPos();
	static float DELTA_Z[] = {HumanCrouchEyeHeight - HumanEyeHeight, -20.0f, 0};
	for (position = 0; position < 2; position++) {
		static float halfHull = 17.0f;
		trace_t result;
		extern ConVar mybot_debug;
		edict_t * ground = BasePlayer(self->getEdict()).getGroundEntity();
		UTIL_TraceHull(pos,
				pos + (target - pos).Normalized() * halfHull,
				Vector(0.0f, -halfHull, DELTA_Z[position]),
				Vector(0.0f, halfHull, 0.0f),
								MASK_NPCSOLID_BRUSHONLY,
								FilterSelf(self->getEdict()->GetIServerEntity(),
										ground == nullptr ? nullptr : ground->GetIServerEntity()),
										&result, mybot_debug.GetBool());
		if (result.DidHit()) {
			break;
		}
	}
	if (position == 2) {
		animationCounter = 90;
	} else if (position == 1) {
		animationCounter = 10;
	} else {
		animationCounter = 0;
	}
	return true;
}

bool DeployMGAction::postCondCheck() {
	if (!armory.getCurrWeapon()->isDeployed()) {
		return false;
	}
	const Player* player = blackboard.getTargetedPlayer();
	Vector end;
	edict_t* target;
	if (player != nullptr) {
		target = player->getEdict();
		end = player->getEyesPos();
	} else {
		target = blackboard.getBlocker();
		if (target == nullptr) {
			return false;
		}
		end = target->GetCollideable()->GetCollisionOrigin();
	}
	return UTIL_IsVisible(end, blackboard, target);
}

bool DeployMGAction::execute() {
	if (armory.getCurrWeapon()->isDeployed()) {
		return true;
	}
	Buttons& buttons = blackboard.getButtons();
	if (position == 2) {
		if (!DodPlayer(blackboard.getSelf()->getEdict()).isProne()
				&& animationCounter == 90) {
			buttons.tap(IN_ALT1);
		}
	} else if (position == 1) {
		buttons.hold(IN_DUCK);
	}
	if (--animationCounter < 0) {
		buttons.tap(IN_ATTACK2);
		return true;
	}
	return false;
}
