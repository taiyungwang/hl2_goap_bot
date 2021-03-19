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

#define PRONE_ANIM_TIME 120

bool DODMGDeployer::execute(Blackboard& blackboard) {
	if (weapon.isDeployed()) {
		animationCounter = -1;
		return true;
	}
	Buttons& buttons = blackboard.getButtons();
	if (--animationCounter < 0) {
		start(blackboard);
		return false;
	}
	if (proneRequired) {
		extern ConVar mybot_var;
		if (DodPlayer(blackboard.getSelf()->getEdict()).isProne()
				&& blackboard.getAimAccuracy(blackboard.getViewTarget()) >= mybot_var.GetFloat()) {
			buttons.tap(IN_ATTACK2);
		}
	}
	return false;
}

void DODMGDeployer::start(Blackboard& blackboard) {
	const Player* self = blackboard.getSelf();
	if (blackboard.getTargetedPlayer() == nullptr
			&& blackboard.getBlocker() == nullptr) {
		blackboard.lookStraight();
	}
	Vector pos(self->getCurrentPosition() + (blackboard.getViewTarget() - self->getCurrentPosition()).Normalized() * HalfHumanWidth);
	pos.z += StepHeight;
	trace_t result;
	extern ConVar mybot_debug;
	edict_t * ground = BasePlayer(self->getEdict()).getGroundEntity();
	UTIL_TraceHull(pos, pos, Vector(0.0f, -HalfHumanWidth, 0.0f),
					Vector(0.0f, HalfHumanWidth, 0.0f),
									MASK_NPCSOLID_BRUSHONLY, FilterSelf(self->getEdict()->GetIServerEntity()),
											&result, mybot_debug.GetBool());
	proneRequired = !result.DidHit();
	blackboard.getButtons().tap(proneRequired ? IN_ALT1 : IN_ATTACK2);
	animationCounter = proneRequired ? PRONE_ANIM_TIME : 5.0f;
}

