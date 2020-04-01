#include "DODMGDeployer.h"

#include <mods/dod/util/DodPlayer.h>
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <player/Bot.h>
#include <weapon/Weapon.h>
#include <weapon/DeployableWeaponBuilder.h>
#include <nav_mesh/nav.h>
#include <util/UtilTrace.h>
#include <util/BasePlayer.h>
#include <in_buttons.h>

#define PRONE_ANIM 120

bool DODMGDeployer::execute(Blackboard& blackboard) {
	if (animationCounter < 0) {
		reset(blackboard);
	}
	if (weapon.isDeployed()) {
		animationCounter = -1;
		return true;
	}
	extern ConVar mybot_var;
	if (blackboard.getAimAccuracy(blackboard.getViewTarget()) < mybot_var.GetFloat()) {
		return false;
	}
	Buttons& buttons = blackboard.getButtons();
	if (position == 2) {
		if (!DodPlayer(blackboard.getSelf()->getEdict()).isProne()
				&& animationCounter == PRONE_ANIM) {
			if (blackboard.getTargetedPlayer() == nullptr
					&& blackboard.getBlocker() == nullptr) {
				blackboard.lookStraight();
			}
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

void DODMGDeployer::reset(Blackboard& blackboard) {
	const Player* self = blackboard.getSelf();
	Vector pos = self->getEyesPos();
	static float DELTA_Z[] = {HumanCrouchEyeHeight - HumanEyeHeight, -20.0f};
	for (position = 0; position < 2; position++) {
		static float halfHull = 17.0f;
		trace_t result;
		extern ConVar mybot_debug;
		edict_t * ground = BasePlayer(self->getEdict()).getGroundEntity();
		UTIL_TraceHull(pos,
				pos + (blackboard.getViewTarget() - pos).Normalized() * halfHull,
				Vector(0.0f, -halfHull, DELTA_Z[position]),
				Vector(0.0f, halfHull, 0.0f),
								MASK_NPCSOLID_BRUSHONLY,
								FilterSelf(self->getEdict()->GetIServerEntity()),
										&result, mybot_debug.GetBool());
		if (result.DidHit()) {
			break;
		}
	}
	if (position == 2) {
		animationCounter = PRONE_ANIM;
	} else if (position == 1) {
		animationCounter = 10;
	} else {
		animationCounter = 0;
	}
}

