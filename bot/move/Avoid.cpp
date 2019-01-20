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
#include <util/EntityUtils.h>
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
	if (checkStuck(pos)) {
		if (!ctx.nextGoalIsLadderStart() && blackboard.isOnLadder()) {
			return new MoveLadder(ctx);
		}
		if (dynamic_cast<Stopped*>(nextState) != nullptr) {
			ctx.setStuck(true);
		}
		if (blackboard.getBlocker() != nullptr) {
			delete nextState;
			return new Stopped(ctx);
		}
		return nextState;
	}
	trace(result, mybot_trace_move_factor.GetFloat());
	Vector goal = ctx.getGoal();
	if (!result.startsolid && result.fraction < 1.0f) {
		if (result.plane.normal.Length() <= 0.0f) {
			result.plane.normal
			= (reinterpret_cast<IServerEntity*>(result.m_pEnt)
			->GetNetworkable()->GetEdict()->GetCollideable()->GetCollisionOrigin()
			- result.endpos).Normalized();
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
	pos.z += 5.0f;
	float halfHull = 17.0f;
	Vector heading = ctx.getGoal();
	heading.z += 5.0f;
	heading -= pos;
	extern ConVar mybot_debug;
	edict_t* ground =
			BasePlayer(self->getEdict()).getGroundEntity();
	UTIL_TraceHull(pos,
			pos + heading.Normalized() * min(heading.Length(), dist - 6.0f),
			Vector(0.0f, -halfHull, 0.0f),
			Vector(halfHull, halfHull,
					self->getEyesPos().DistTo(pos)
							+ (ctx.getGoal() - pos).Normalized().z * halfHull),
							MASK_NPCSOLID_BRUSHONLY,
							FilterSelfAndTarget(blackboard.getSelf()->getEdict()->GetIServerEntity(),
									ground == nullptr ? nullptr : ground->GetIServerEntity()),
									&result, mybot_debug.GetBool());
	edict_t* blocker =
			result.m_pEnt == nullptr ?
					nullptr :
					reinterpret_cast<IServerEntity*>(result.m_pEnt)->GetNetworkable()->GetEdict();
	blackboard.setBlocker(blocker != nullptr &&
					(Q_stristr(blocker->GetClassName(), "breakable") != nullptr
					|| Q_stristr(blocker->GetClassName(), "physics") != nullptr) ?
					blocker: nullptr);
	if (blackboard.getBlocker() == nullptr && blocker != nullptr) {
		auto& players = blackboard.getPlayers();
		int team = self->getTeam();
		FOR_EACH_MAP_FAST(players, i) {
			if (blocker == players[i]->getEdict()
					&& (team <= 0 || team != players[i]->getTeam())) {
				blackboard.setBlocker(blocker);
				break;
			}
		}
	}
}

