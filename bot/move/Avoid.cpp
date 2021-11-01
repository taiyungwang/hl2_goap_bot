#include "Avoid.h"

#include "Stopped.h"
#include "MoveLadder.h"
#include "MoveStateContext.h"
#include "StepLeft.h"
#include "Jump.h"
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <player/Bot.h>
#include <weapon/Weapon.h>
#include <nav_mesh/nav_entities.h>
#include <util/UtilTrace.h>
#include <util/EntityUtils.h>
#include <utlmap.h>
#include <eiface.h>
#include <in_buttons.h>

using namespace std;

extern IVEngineServer* engine;

extern ConVar nav_slope_limit;

const static float MIN_DIST = 0.00001f;

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

class IgnoreSelfAndTeamWall: public CTraceFilter {
public:
	IgnoreSelfAndTeamWall(edict_t* self): self(self) {
	}

	virtual ~IgnoreSelfAndTeamWall() {
	}

	bool ShouldHitEntity(IHandleEntity *pHandleEntity,
			int contentsMask) override {
		edict_t* hit = reinterpret_cast<IServerUnknown*>(const_cast<IHandleEntity*>(pHandleEntity))
						->GetNetworkable()->GetEdict();
		return hit != self && string(hit->GetClassName()).find("func_team") == string::npos;
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
	bool crouching = ctx.getType() & NAV_MESH_CROUCH;
	Vector goal = ctx.getGoal();
	const trace_t& result = ctx.trace(goal, crouching);
	edict_t* currBlocker = getEdict(result);
	const char* currBlockerName = currBlocker == nullptr || currBlocker->IsFree() ? nullptr: currBlocker->GetClassName();
	int team = blackboard.getSelf()->getTeam();
	edict_t* self = blackboard.getSelf()->getEdict();
	if (ctx.isStuck()) {
		ctx.setStuck(false);
		if (blackboard.isOnLadder()) {
			if (ctx.nextGoalIsLadderStart()) {
				return new MoveLadder(ctx);
			}
			return nextState;
		}
		extern CGlobalVars *gpGlobals;
		int idx = currBlocker == nullptr ? -1 : engine->IndexOfEdict(currBlocker);
		if ((idx > 0 && idx <= gpGlobals->maxClients)
						|| Q_stristr(currBlockerName, "physics") != nullptr
						|| Q_stristr(currBlockerName, "breakable") != nullptr
						|| Q_stristr(currBlockerName, "func_team") != nullptr) {
			blocker = currBlocker;
			if (Q_stristr(blocker->GetClassName(), "func_team") != nullptr
				// ensure that nothing else is blocking us.
				&& !ctx.trace(pos, goal, crouching, IgnoreSelfAndTeamWall(self)).DidHit()) {
					setTeamWall(blocker, team);
			}
		}
		auto& players = Player::getPlayers();
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
		if (dynamic_cast<Stopped*>(nextState) != nullptr) {
			// completely stuck.
			ctx.setStuck(true);
		}
		return nextState;
	}
	if (currBlocker != nullptr) {
		Vector blockerPos = currBlocker->GetCollideable()->GetCollisionOrigin();
		if (Q_stristr(currBlockerName, "func_team") != nullptr) {
			Extent wallExtent;
			wallExtent.Init(currBlocker);
			if (wallExtent.Contains(pos)) {
				// set team wall against other team if we can walk through.
				setTeamWall(currBlocker, (team + 1) % 2);
			}
		}
		if ((Q_stristr(currBlockerName, "physics") != nullptr
				|| Q_stristr(currBlockerName, "breakable") != nullptr)
				&& pos.DistTo(blockerPos) < Weapon::MELEE_RANGE) {
			blackboard.setBlocker(currBlocker);
		}
	}
	if (result.DidHit() && currBlocker != nullptr
			&& Q_stristr(currBlockerName, "func_team") == nullptr
			&& (result.plane.normal.LengthSqr() == 0.0f
					|| result.plane.normal.z > nav_slope_limit.GetFloat())) {
		auto collideable = currBlocker->GetCollideable();
		Vector avoid(collideable->GetCollisionOrigin());
		float dist = Max(MIN_DIST, result.endpos.DistTo(pos));
		if (!result.startsolid && avoid.LengthSqr() == 0.0f) {
			Vector mins, maxs;
			collideable->WorldSpaceSurroundingBounds(&mins, &maxs);
			avoid = (maxs + mins) / 2.0f;
		}
		if (avoid.LengthSqr() > 0.0f && !result.startsolid) {
			avoid = result.endpos - avoid;
		} else {
			Vector traceTarget = goal;
			traceTarget.z += StepHeight;
			if (result.startsolid) {
				traceTarget = (traceTarget - pos).Normalized() * HalfHumanWidth * 2.0f + pos;
			}
			Vector dir = perpLeft2D(traceTarget, pos).Normalized() * HalfHumanWidth;
			avoid = dir + traceTarget;
			dist = trace(pos, avoid);
			dir = inverse2D(dir);
			if (trace(pos, dir + traceTarget) < dist) {
				avoid = dir + traceTarget;
			}
			avoid = pos - avoid - traceTarget;
		}
		goal += avoid.Normalized() * HalfHumanWidth;
	}
	moveStraight(goal);
	return nullptr;
}

void Avoid::setTeamWall(edict_t *blocker, int team) {
	// assume the first team that gets block correctly identifies the team blocker.
	extern CUtlMap<int, NavEntity*> blockers;
	int idx = engine->IndexOfEdict(blocker);
	if (blockers.IsValidIndex(blockers.Find(idx))) {
		return;
	}
	CFuncNavBlocker* navBlocker = new CFuncNavBlocker(blocker);
	navBlocker->setBlockedTeam(team);
	blockers.Insert(idx, navBlocker);
}

float Avoid::trace(const Vector& pos, const Vector& goal) const {
	auto &blackboard = ctx.getBlackboard();
	FilterList filter;
	filter.add(blackboard.getSelf()->getEdict()).add(blackboard.getTarget());
	trace_t tr;
	for (;;) {
		UTIL_TraceLine(pos, goal, MASK_PLAYERSOLID, &filter, &tr);
		if (!tr.startsolid) {
			break;
		}
		filter.add(getEdict(tr));
	}
	if (!tr.DidHit() || (tr.plane.normal.LengthSqr() > 0.0f
			&& tr.plane.normal.z <= nav_slope_limit.GetFloat())) {
		return goal.DistTo(pos);
	}
	return tr.startsolid ? MIN_DIST : Max(MIN_DIST, pos.DistTo(tr.endpos));
}
