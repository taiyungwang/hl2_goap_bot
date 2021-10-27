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
#include <util/BasePlayer.h>
#include <eiface.h>
#include <in_buttons.h>

using namespace std;

// how much should blocker push back against our movement.
static ConVar mybot_avoid_move_factor("mybot_avoid_move_factor", "1");

static edict_t* getEdict(const trace_t& result) {
	if (result.m_pEnt == nullptr) {
		return nullptr;
	}
	extern IServerGameEnts *servergameents;
	return servergameents->BaseEntityToEdict(result.m_pEnt);
}

class IgnoreAllButPhysicsAndBreakable: public CTraceFilterEntitiesOnly {
public:
	virtual ~IgnoreAllButPhysicsAndBreakable() {
	}

	bool ShouldHitEntity(IHandleEntity *pHandleEntity,
			int contentsMask) override {
		string name(reinterpret_cast<IServerUnknown*>(const_cast<IHandleEntity*>(pHandleEntity))
				->GetNetworkable()->GetEdict()->GetClassName());
		return name.find("physics") != string::npos
				|| name.find("breakable") != string::npos;
	}

};

class IgnoreSelfAndTeamWall: public CTraceFilterEntitiesOnly {
public:
	IgnoreSelfAndTeamWall(edict_t* self): self(self) {
	}

	virtual ~IgnoreSelfAndTeamWall() {
	}

	bool ShouldHitEntity(IHandleEntity *pHandleEntity,
			int contentsMask) override {
		edict_t* hit = reinterpret_cast<IServerUnknown*>(const_cast<IHandleEntity*>(pHandleEntity))
						->GetNetworkable()->GetEdict();
		string name(hit->GetClassName());
		return hit != self && name.find("func_team") == string::npos;
	}

private:
	edict_t* self;
};

MoveState* Avoid::move(const Vector& pos) {
	if (ctx.reachedGoal()) {
		ctx.setTargetOffset(0);
		delete nextState;
		return new Stopped(ctx);
	}
	Blackboard& blackboard = ctx.getBlackboard();
	const trace_t& result = ctx.getTraceResult();
	Vector goal = ctx.getGoal();
	edict_t* currBlocker = getEdict(result);
	bool crouching = ctx.getType() & NAV_MESH_CROUCH;
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
				&& blocker->GetCollideable() != nullptr
				&& Q_stristr(currBlockerName, "func_team") == nullptr) {
			if (Q_stristr(blocker->GetClassName(), "player") != nullptr
				&& ctx.trace(pos, goal, crouching,
						IgnoreAllButPhysicsAndBreakable()).DidHit()) {
				blocker = getEdict(result);
			}
			blackboard.setBlocker(blocker);
		}
		if (result.startsolid || dynamic_cast<Stopped*>(nextState) != nullptr) {
			// completely stuck.
			ctx.setStuck(true);
			extern CUtlMap<int, NavEntity*> blockers;
			idx = engine->IndexOfEdict(blocker);
			if (blocker != nullptr && Q_stristr(blocker->GetClassName(), "func_team") != nullptr
					&& !blockers.IsValidIndex(blockers.Find(idx))) {
				// ensure that nothing else is blocking us.
				edict_t* self = blackboard.getSelf()->getEdict();
				if (!ctx.trace(pos, goal, crouching,
						IgnoreSelfAndTeamWall(self)).DidHit()) {
					// assume the first team that gets block correctly identifies the team blocker.
					CFuncNavBlocker* navBlocker = new CFuncNavBlocker(blocker);
					navBlocker->setBlockedTeam(team);
					blockers.Insert(idx, navBlocker);
				} else {
					blackboard.setBlocker(getEdict(result));
				}
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
