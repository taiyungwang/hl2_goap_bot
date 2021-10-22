#include "Avoid.h"

#include "Stopped.h"
#include "MoveLadder.h"
#include "MoveStateContext.h"
#include "StepLeft.h"
#include "Jump.h"
#include "MoveTraceFilter.h"
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <player/Bot.h>
#include <nav_mesh/nav_entities.h>
#include <util/UtilTrace.h>
#include <util/EntityUtils.h>
#include <util/BasePlayer.h>
#include <eiface.h>
#include <in_buttons.h>

// how much should blocker push back against our movement.
static ConVar mybot_avoid_move_factor("mybot_avoid_move_factor", "1");

static edict_t* getEdict(const trace_t& result) {
	if (result.m_pEnt == nullptr) {
		return nullptr;
	}
	extern IServerGameEnts *servergameents;
	return servergameents->BaseEntityToEdict(result.m_pEnt);
}

MoveState* Avoid::move(const Vector& pos) {
	if (ctx.reachedGoal()) {
		ctx.setTargetOffset(0);
		delete nextState;
		return new Stopped(ctx);
	}
	Blackboard& blackboard = ctx.getBlackboard();
	ctx.trace(ctx.getGoal(), ctx.getType() & NAV_MESH_CROUCH);
	trace_t& result = ctx.getTraceResult();
	Vector goal = ctx.getGoal();
	MoveTraceFilter filter(*blackboard.getSelf(), blackboard.getTarget());
	edict_t* currBlocker = getEdict(result);
	const char* currBlockerName = currBlocker == nullptr || currBlocker->IsFree() ? nullptr: currBlocker->GetClassName();
	if (ctx.isStuck()) {
		ctx.setStuck(false);
		if (blackboard.isOnLadder()) {
			if (ctx.nextGoalIsLadderStart()) {
				return new MoveLadder(ctx);
			}
			return nextState;
		}
		extern IVEngineServer* engine;
		extern CGlobalVars *gpGlobals;
		int idx = currBlocker == nullptr ? -1 : engine->IndexOfEdict(currBlocker);
		if ((idx > 0 && idx <= gpGlobals->maxClients)
						|| Q_stristr(currBlockerName, "physics") != nullptr
						|| Q_stristr(currBlockerName, "breakable") != nullptr
						|| Q_stristr(currBlockerName, "func_team") != nullptr) {
			blocker = currBlocker;
		}
		auto& players = Player::getPlayers();
		int team = blackboard.getSelf()->getTeam();
		if (blocker != nullptr && !blocker->IsFree()
				&& blocker->GetCollideable() != nullptr) {
			const char* blockerName = blocker->GetClassName();
			if (Q_stristr(blockerName, "physics") != nullptr
					|| Q_stristr(blockerName, "breakable") != nullptr) {
				blackboard.setBlocker(blocker);
			} else {
				int idx = engine->IndexOfEdict(blocker);
				if (idx <= gpGlobals->maxClients) {
					auto player = players.find(idx);
					if (team < 2 || (player != players.end()
							&& player->second->getTeam() != blackboard.getSelf()->getTeam())) {
						blackboard.setBlocker(blocker);
					}
				}
			}
		}
		if (result.startsolid || dynamic_cast<Stopped*>(nextState) != nullptr) {
			// completely stuck.
			ctx.setStuck(true);
			extern CUtlMap<int, NavEntity*> blockers;
			int edictId = engine->IndexOfEdict(blocker);
			if (blocker != nullptr && Q_stristr(blocker->GetClassName(), "func_team") != nullptr
					&& !blockers.IsValidIndex(blockers.Find(edictId))) {
				// assume the first team that gets block correctly identifies the team blocker.
				CFuncNavBlocker* navBlocker = new CFuncNavBlocker(blocker);
				navBlocker->setBlockedTeam(team);
				blockers.Insert(edictId, navBlocker);
			}
		}
		return nextState;
	}
	extern ConVar nav_slope_limit;
	// TODO: the plane / world spawn avoidance doesn't really work, since using the plane
	// as a avoidance adjustment doesn't necessarily push the bot toward the right direction.
	if (result.DidHit() && !result.startsolid
			&& currBlocker != nullptr
			&& Q_stristr(currBlockerName, "func_team") == nullptr
			&& (result.plane.normal.LengthSqr() == 0.0f
					|| result.plane.normal.z > nav_slope_limit.GetFloat())) {
		Vector avoid(currBlocker->GetCollideable()->GetCollisionOrigin());
		avoid = avoid.x == 0.0f && avoid.y == 0.0f && avoid.z == 0.0f
			? result.plane.normal: (result.endpos - avoid).Normalized();
		goal += avoid / Max(0.000001f, result.endpos.DistTo(pos))
				* mybot_avoid_move_factor.GetFloat();
	}
	moveStraight(goal);
	return nullptr;
}
