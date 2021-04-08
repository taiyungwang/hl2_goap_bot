#include "DODMGDeployer.h"

#include <mods/dod/util/DodPlayer.h>
#include <move/MoveStateContext.h>
#include <move/Navigator.h>
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <player/Bot.h>
#include <player/Vision.h>
#include <weapon/Weapon.h>
#include <weapon/DeployableWeaponBuilder.h>
#include <nav_mesh/nav.h>
#include <nav_mesh/nav_area.h>
#include <util/UtilTrace.h>
#include <util/BasePlayer.h>
#include <in_buttons.h>

#define PRONE_ANIM_TIME 120

DODMGDeployer::~DODMGDeployer() {
	if (moveCtx != nullptr) {
		delete moveCtx;
	}
}

bool DODMGDeployer::execute(Blackboard& blackboard) {
	if (!started) {
		start(blackboard);
		return false;
	}
	if (weapon.isDeployed() || --animationCounter < 0) {
		started = false;
		return true;
	}
	if (proneRequired) {
		extern ConVar mybot_var;
		auto self = blackboard.getSelf();
		edict_t* selfEnt = blackboard.getSelf()->getEdict();
		if (DodPlayer(selfEnt).isProne()) {
			Buttons& buttons = blackboard.getButtons();
			if (target != nullptr && !target->isDead()) {
				trace_t result;
				UTIL_IsVisible(result, self->getEyesPos(), blackboard, target->getEdict());
				extern ConVar nav_slope_limit;
				if (result.DidHit()) {
					if (result.endpos.DistTo(result.startpos) > HalfHumanWidth
							|| (result.plane.normal.LengthSqr() > 0 && result.plane.normal.z <= nav_slope_limit.GetFloat())) {
						// TODO: hacky
						if (moveCtx == nullptr) {
							moveCtx = new MoveStateContext(blackboard);
						}
						moveCtx->setGoal(result.endpos);
						CNavArea* area = Navigator::getArea(selfEnt, self->getTeam());
						moveCtx->traceMove(false);
						moveCtx->move(area == nullptr ? NAV_MESH_INVALID: area->GetAttributes());
						animationCounter++;
					} else {
						buttons.tap(IN_ALT1);
						start(blackboard);
					}
					return false;
				}
			}
			if (blackboard.getAimAccuracy(blackboard.getViewTarget()) >= mybot_var.GetFloat()) {
				buttons.tap(IN_ATTACK2);
			}
		}
	}
	return false;
}

void DODMGDeployer::undeploy(Blackboard& blackboard) {
	Deployer::undeploy(blackboard);
	animationCounter = -1;
}

void DODMGDeployer::start(Blackboard& blackboard) {
	const Player* self = blackboard.getSelf();
	target = blackboard.getTargetedPlayer();
	if (target == nullptr && blackboard.getBlocker() == nullptr) {
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
	started = true;
	blackboard.getButtons().tap(proneRequired ? IN_ALT1 : IN_ATTACK2);
	animationCounter = proneRequired ? PRONE_ANIM_TIME : 5.0f;
}

