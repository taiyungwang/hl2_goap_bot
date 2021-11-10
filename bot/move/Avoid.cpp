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
#include <util/BasePlayer.h>
#include <utlmap.h>
#include <eiface.h>
#include <in_buttons.h>

using namespace std;

extern IVEngineServer* engine;

extern ConVar nav_slope_limit;

const static float MIN_DIST = 0.00001f;

static ConVar maxStuckFrames("mybot_max_stuck_frames", "3");
static ConVar minStuckSpeed("mybot_min_stuck_speed", "2.0f");

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
		string name(entityFromEntityHandle(pHandleEntity)->GetClassName());
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
		edict_t* hit = entityFromEntityHandle(pHandleEntity);
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
	const char* currBlockerName = currBlocker == nullptr || currBlocker->IsFree()
			? nullptr : currBlocker->GetClassName();
	int team = blackboard.getSelf()->getTeam();
	edict_t* self = blackboard.getSelf()->getEdict();
	if (blackboard.isOnLadder()) {
		if (ctx.nextGoalIsLadderStart()) {
			delete nextState;
			nextState = nullptr;
			return new MoveLadder(ctx);
		}
		return nextState;
	}
	float speed = BasePlayer(self).getVelocity().Length();
	if (!(ctx.getType() & NAV_MESH_CROUCH) && speed < minStuckSpeed.GetFloat()) {
		stuckFrames++;
	} else {
		stuckFrames = 0;
	}
	if (stuckFrames > maxStuckFrames.GetInt()) {
		ctx.setStuck(true);
	}
	if (ctx.isStuck()) {
		extern CGlobalVars *gpGlobals;
		int idx = currBlocker == nullptr ? -1 : engine->IndexOfEdict(currBlocker);
		if (currBlocker != nullptr) {
			if (Q_stristr(currBlockerName, "func_team") != nullptr) {
				// ensure that nothing else is blocking us.
				if (!ctx.trace(pos, goal, crouching, IgnoreSelfAndTeamWall(self)).DidHit()
						|| (Q_stristr(getEdict(result)->GetClassName(), "worldspawn") != nullptr
								&& result.plane.normal.z < nav_slope_limit.GetFloat())) {
					setTeamWall(currBlocker, team);
				}
			} else if (Q_stristr(currBlockerName, "physics") != nullptr
					|| Q_stristr(currBlockerName, "breakable") != nullptr
					|| (Q_stristr(currBlockerName, "player") != nullptr
							&& ctx.trace(pos, goal, crouching,
									IgnoreAllButPhysicsAndBreakable()).DidHit())) {
				blackboard.setBlocker(currBlocker);
			}
		}
		if (dynamic_cast<Stopped*>(nextState) == nullptr) {
			// try to get unstuck.
			ctx.setStuck(false);
		}
		return nextState;
	}
	if (Q_stristr(currBlockerName, "func_team") != nullptr) {
		blocker = currBlocker;
	}
	if (blocker != nullptr
			&& Q_stristr(blocker->GetClassName(), "func_team") != nullptr) {
		Extent wallExtent;
		wallExtent.Init(blocker);
		if (wallExtent.Contains(pos)) {
			// set team wall against other team if we can walk through.
			setTeamWall(blocker, (team + 1) % 2);
		}
	}
	if (currBlocker != nullptr
			&& Q_stristr(currBlockerName, "func_team") == nullptr
			&& (Q_stristr(currBlockerName, "worldspawn") == nullptr
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
		goal += avoid.Normalized() * MoveStateContext::TARGET_OFFSET;
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

/**
 * A filter that maintains a list of entities to ignore;
 */
class FilterList: public CTraceFilter {
public:
	virtual ~FilterList() {
	}

	/**
	 * Adds a new entity to the list.  Null values are ignored.
	 */
	FilterList& add(edict_t* ignore) {
		if (ignore != nullptr) {
			this->ignore.push_back(ignore->GetIServerEntity());
		}
		return *this;
	}

	bool ShouldHitEntity(IHandleEntity *pHandleEntity,
			int contentsMask) override {
		for (auto handle: ignore) {
			if (pHandleEntity == handle) {
				return false;
			}
		}
		return true;
	}

protected:
	list<IHandleEntity*> ignore;
};


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
