#include "Avoid.h"

#include "Stopped.h"
#include "MoveLadder.h"
#include "MoveStateContext.h"
#include "StepLeft.h"
#include "Jump.h"
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <player/Bot.h>
#include <util/UtilTrace.h>
#include <util/BasePlayer.h>
#include <eiface.h>
#include <in_buttons.h>

// how far ahead to look for blockers
static ConVar mybot_trace_move_factor("mybot_trace_move_factor", "30.0f");
// how much should blocker push back against our movement.
static ConVar mybot_avoid_move_factor("mybot_avoid_move_factor", "10.0f");


Avoid::Avoid(MoveStateContext& ctx, MoveState* nextState) :
		MoveState(ctx), nextState(nextState) {
	ctx.setLadderDir(CNavLadder::NUM_LADDER_DIRECTIONS);
}

MoveState* Avoid::move(const Vector& pos) {
	if (ctx.reachedGoal()) {
		delete nextState;
		return new Stopped(ctx);
	}
	Blackboard& blackboard = ctx.getBlackboard();
	trace_t result;
	trace(result, mybot_trace_move_factor.GetFloat());
	ctx.setBlocker(result.m_pEnt == nullptr ?
					nullptr :
					reinterpret_cast<IServerEntity*>(result.m_pEnt)->GetNetworkable()->GetEdict());
	if (checkStuck(pos)) {
		if (!ctx.nextGoalIsLadderStart() && blackboard.isOnLadder()) {
			return new MoveLadder(ctx);
		}
		if (dynamic_cast<Stopped*>(nextState) != nullptr) {
			ctx.setStuck(true);
		}
		return nextState;
	}
	Vector goal = ctx.getGoal();
	if (!result.startsolid && result.fraction < 1.0f) {
		if (result.plane.normal.Length() <= 0.0f) {
			result.plane.normal
			= (result.endpos - reinterpret_cast<IServerEntity*>(result.m_pEnt)
			->GetNetworkable()->GetEdict()->GetCollideable()->GetCollisionOrigin()).Normalized();
		}
		goal += result.plane.normal * mybot_avoid_move_factor.GetFloat()
				/ max(0.01f, result.startpos.DistTo(result.endpos));
	}
	moveStraight(goal);
	return nullptr;
}

void Avoid::trace(CGameTrace& result, float dist) const {
	Blackboard& blackboard = ctx.getBlackboard();
	const Player* self = blackboard.getSelf();
	Vector pos = self->getCurrentPosition();
	pos.z += StepHeight;
	static float halfHull = 17.0f;
	Vector heading = ctx.getGoal();
	heading.z += StepHeight;
	heading -= pos;
	extern ConVar mybot_debug;
	edict_t* ground = BasePlayer(self->getEdict()).getGroundEntity();
	UTIL_TraceHull(pos,
			pos + heading.Normalized() * min(heading.Length(), dist - 6.0f),
			Vector(0.0f, -halfHull, 0.0f),
			Vector(halfHull, halfHull,
					self->getEyesPos().DistTo(pos)
							+ (ctx.getGoal() - pos).Normalized().z * halfHull),
							MASK_NPCSOLID_BRUSHONLY,
							FilterSelfAndTarget(self->getEdict()->GetIServerEntity(),
									ground == nullptr ? nullptr : ground->GetIServerEntity()),
									&result, mybot_debug.GetBool());
}

