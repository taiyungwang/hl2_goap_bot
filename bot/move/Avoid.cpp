#include "Avoid.h"

#include "Stopped.h"
#include "MoveLadder.h"
#include "MoveStateContext.h"
#include "StepLeft.h"
#include "Jump.h"
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <player/Bot.h>
#include <nav_mesh/nav_entities.h>
#include <util/UtilTrace.h>
#include <util/EntityUtils.h>
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
		delete nextState;
		return new Stopped(ctx);
	}
	Blackboard& blackboard = ctx.getBlackboard();
	const trace_t& result = ctx.getTraceResult();
	edict_t* currBlocker = getEdict(result);
	const char* currBlockerName = currBlocker == nullptr || currBlocker->IsFree() ? nullptr: currBlocker->GetClassName();
	if (ctx.checkStuck()) {
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
					auto i = players.Find(idx);
					if (team < 2 || (players.IsValidIndex(i)
							&& players[i]->getTeam() != team)) {
						blackboard.setBlocker(blocker);
					}
				}
			}
		}
		if (blackboard.isOnLadder()) {
			if (ctx.nextGoalIsLadderStart()) {
				return new MoveLadder(ctx);
			}
			return new Jump(ctx);
		}
		if (dynamic_cast<Stopped*>(nextState) != nullptr) {
			// completely stuck.
			if (blocker != nullptr && !blocker->IsFree()
					&& blocker->GetCollideable() != nullptr) {
				const char* blockerName = blocker->GetClassName();
				if (Q_stristr(blockerName, "physics") != nullptr
						|| Q_stristr(blockerName, "breakable") != nullptr) {
					blackboard.setBlocker(blocker);
				} else if (Q_stristr(blocker->GetClassName(), "func_team") != nullptr) {
					extern CUtlMap<int, NavEntity*> blockers;
					CFuncNavBlocker* navBlocker = new CFuncNavBlocker(blocker);
					navBlocker->setBlockedTeam(team);
					blockers.Insert(engine->IndexOfEdict(blocker), navBlocker);
				} else {
					int idx = engine->IndexOfEdict(blocker);
					if (idx <= gpGlobals->maxClients) {
						auto i = players.Find(idx);
						if (team < 2|| (players.IsValidIndex(i)
								&& players[i]->getTeam() != blackboard.getSelf()->getTeam())) {
							blackboard.setBlocker(blocker);
						}
					}
				}
			}
			ctx.setStuck(true);
		}
		return nextState;
	}
	Vector goal = ctx.getGoal();
	extern ConVar nav_slope_limit;
	if (!result.startsolid && result.DidHit()
			&& currBlocker != nullptr
			&& Q_stristr(currBlockerName, "func_team") == nullptr
			&& goal.AsVector2D().DistTo(result.endpos.AsVector2D()) > HalfHumanWidth
			&& (result.plane.normal.LengthSqr() == 0.0f
			|| result.plane.normal.z > nav_slope_limit.GetFloat())) {
		Vector avoid = currBlocker->GetCollideable()->GetCollisionOrigin();
		avoid = avoid.x == 0.0f && avoid.y == 0.0f && avoid.z == 0.0f ?
			result.plane.normal : (result.endpos - avoid).Normalized();
		goal = result.endpos + avoid / Max(0.000001f, (result.endpos - pos).Length())
				* mybot_avoid_move_factor.GetFloat();
	}
	moveStraight(goal);
	return nullptr;
}


