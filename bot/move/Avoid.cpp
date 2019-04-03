#include "Avoid.h"

#include "Stopped.h"
#include "MoveLadder.h"
#include "MoveStateContext.h"
#include "StepLeft.h"
#include "Jump.h"
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <player/Bot.h>
#include <navmesh/nav_entities.h>
#include <util/UtilTrace.h>
#include <util/EntityUtils.h>
#include <eiface.h>
#include <in_buttons.h>

// how much should blocker push back against our movement.
static ConVar mybot_avoid_move_factor("mybot_avoid_move_factor", "5");

static edict_t* getEdict(const trace_t& result) {
	return result.m_pEnt == nullptr ?
					nullptr :
					reinterpret_cast<IServerEntity*>(result.m_pEnt)->GetNetworkable()->GetEdict();
}

MoveState* Avoid::move(const Vector& pos) {
	if (ctx.reachedGoal()) {
		delete nextState;
		return new Stopped(ctx);
	}
	Blackboard& blackboard = ctx.getBlackboard();
	const trace_t& result = ctx.getTraceResult();
	edict_t* currBlocker = getEdict(result);
	const char* currBlockerName = currBlocker == nullptr ? nullptr: currBlocker->GetClassName();
	extern IVEngineServer* engine;
	extern CGlobalVars *gpGlobals;
	int idx = currBlocker == nullptr ? -1 : engine->IndexOfEdict(currBlocker);
	if (currBlocker != nullptr
			&& ((idx > 0 && idx <= gpGlobals->maxClients)
					|| Q_stristr(currBlockerName, "physics") != nullptr
					|| Q_stristr(currBlockerName, "breakable") != nullptr
					|| Q_stristr(currBlockerName, "func_team") != nullptr)) {
		blocker = currBlocker;
	}
	if (checkStuck(pos, ctx.getGoal())) {
		if (blocker != nullptr && !blocker->IsFree()
				&& blocker->GetCollideable() != nullptr) {
			const char* blockerName = blocker->GetClassName();
			if (Q_stristr(blockerName, "physics") != nullptr
					|| Q_stristr(blockerName, "breakable") != nullptr) {
				blackboard.setBlocker(blocker);
			} else if (Q_stristr(blockerName, "func_team") != nullptr) {
				extern CUtlVector<NavEntity*> blockers;
				CFuncNavBlocker* navBlocker = new CFuncNavBlocker(blocker);
				navBlocker->setBlockedTeam(blackboard.getSelf()->getTeam());
				blockers.AddToTail(navBlocker);
			} else {
				int idx = engine->IndexOfEdict(blocker);
				if (idx <= gpGlobals->maxClients) {
					auto i = blackboard.getPlayers().Find(idx);
					if (blackboard.getSelf()->getTeam() < 2
							|| (blackboard.getPlayers().IsValidIndex(i)
							&& blackboard.getPlayers()[i]->getTeam() != blackboard.getSelf()->getTeam())) {
						blackboard.setBlocker(blocker);
					}
				}
			}
			ctx.setStuck(true);
			return new Stopped(ctx);
		}
		if (blackboard.isOnLadder()) {
			if (ctx.nextGoalIsLadderStart()) {
				return new MoveLadder(ctx);
			}
			return new Jump(ctx);
		}
		if (dynamic_cast<Stopped*>(nextState) != nullptr) {
			ctx.setStuck(true);
		}
		return nextState;
	}
	Vector goal = ctx.getGoal();
	if (!result.startsolid && result.DidHit()
			&& currBlocker != nullptr
			&& Q_stristr(currBlockerName, "func_team") == nullptr) {
		Vector avoid = currBlocker->GetCollideable()->GetCollisionOrigin();
		avoid = avoid.x == 0.0f && avoid.y == 0.0f && avoid.z == 0.0f ?
			result.plane.normal : (result.endpos - avoid).Normalized();
		goal = result.endpos + avoid * mybot_avoid_move_factor.GetFloat();
	}
	moveStraight(goal);
	return nullptr;
}


