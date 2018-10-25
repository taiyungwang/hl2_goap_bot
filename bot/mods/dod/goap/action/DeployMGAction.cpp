#include "DeployMGAction.h"

#include <mods/dod/util/DODPlayer.h>
#include <player/Blackboard.h>
#include <player/Player.h>
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
	float z = self->getCurrentPosition().z + 1.0f;
	for (position = 0; position < 3; position++) {
		if (position == 2) {
			break;
		}
		float halfHull = 17.0f;
		trace_t result;
		extern ConVar mybot_debug;
		edict_t * ground = BasePlayer(self->getEdict()).getGroundEntity();
		UTIL_TraceHull(pos,
				pos + (target - pos).Normalized() * halfHull,
				Vector(0.0f, -halfHull, 0.0f),
				Vector(0.0f, halfHull,z),
								MASK_NPCSOLID_BRUSHONLY,
								FilterSelf(self->getEdict()->GetIServerEntity(),
										ground == nullptr ? nullptr : ground->GetIServerEntity()),
										&result, mybot_debug.GetBool());
		if (result.fraction == 1.0f) {
			break;
		}
		z += HumanCrouchHeight;
	}
	blackboard.setViewTarget(target);
	return true;
}

bool DeployMGAction::postCondCheck() {
	return armory.getCurrWeapon()->isDeployed();
}

bool DeployMGAction::execute() {
	if (postCondCheck()) {
		return true;
	}
	Buttons& buttons = blackboard.getButtons();
	if (position == 0) {
		if (!DODPlayer(blackboard.getSelf()->getEdict()).isProne()) {
			buttons.tap(IN_ALT1);
		}
	} else if (position == 1) {
		buttons.hold(IN_DUCK);
	}
	buttons.tap(IN_ATTACK2);
	return false;
}
