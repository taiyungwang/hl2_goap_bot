#include "Avoid.h"

#include "Stopped.h"
#include "MoveLadder.h"
#include "MoveStateContext.h"
#include "StepLeft.h"
#include "Jump.h"
#include <player/Buttons.h>
#include <player/Bot.h>
#include <weapon/Weapon.h>
#include <nav_mesh/nav_entities.h>
#include <util/UtilTrace.h>
#include <util/EntityUtils.h>
#include <util/BasePlayer.h>
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
	bool crouching = ctx.getType() & NAV_MESH_CROUCH;
	Vector goal = ctx.getGoal();
	const trace_t& result = ctx.trace(goal, crouching);
	// if we are stepping off a cliff, and the goal is right at the base of the cliff
	float dist = goal.AsVector2D().DistTo(pos.AsVector2D());
	if (pos.z - goal.z > StepHeight
			&&  dist < HalfHumanWidth) {
		goal += (goal - pos).Normalized() * (HalfHumanWidth + dist);
	}
	edict_t* currBlocker = getEdict(result);
	const char* currBlockerName = currBlocker == nullptr || currBlocker->IsFree()
			? nullptr : currBlocker->GetClassName();
	Bot *self = ctx.getSelf();
	int team = self->getTeam();
	edict_t* selfEnt = self->getEdict();
	if (self->isOnLadder()) {
		if (ctx.nextGoalIsLadderStart()) {
			delete nextState;
			nextState = nullptr;
			return new MoveLadder(ctx);
		}
		self->getButtons().hold(IN_FORWARD);
		self->getButtons().tap(IN_USE);
	}
	float speed = BasePlayer(selfEnt).getVelocity()->Length();
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
		if (currBlocker != nullptr) {
			if (Q_stristr(currBlockerName, "func_team") != nullptr) {
				// ensure that nothing else is blocking us.
				if (!ctx.trace(pos, goal, crouching, IgnoreSelfAndTeamWall(selfEnt)).DidHit()
						|| (Q_stristr(getEdict(result)->GetClassName(), "worldspawn") != nullptr
								&& result.plane.normal.z < nav_slope_limit.GetFloat())) {
					setTeamWall(currBlocker, team);
				}
			} else if (Q_stristr(currBlockerName, "physics") != nullptr
					|| Q_stristr(currBlockerName, "breakable") != nullptr
					|| (Q_stristr(currBlockerName, "player") != nullptr
							&& ctx.trace(pos, goal, crouching,
									IgnoreAllButPhysicsAndBreakable()).DidHit())) {
				self->setBlocker(currBlocker);
			}
		}
		if (dynamic_cast<Stopped*>(nextState) == nullptr) {
			// try to get unstuck.
			ctx.setStuck(false);
		}
		return nextState;
	}
	if (currBlockerName != nullptr && Q_stristr(currBlockerName, "func_team") != nullptr) {
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
	extern std::unordered_map<int, CFuncNavBlocker> blockers;
	int idx = engine->IndexOfEdict(blocker);
	if (blockers.find(idx) == blockers.end()) {
		blockers.emplace(idx, blocker);
		blockers.at(idx).setBlockedTeam(team);
	}
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
			this->ignore.insert(ignore->GetIServerEntity());
		}
		return *this;
	}

	bool ShouldHitEntity(IHandleEntity *pHandleEntity,
			int contentsMask) override {
		bool shouldHit = ignore.find(pHandleEntity) == ignore.end();
		if (shouldHit) {
			ignore.insert(pHandleEntity);
		}
		return shouldHit;
	}

	size_t getIgnoreSize() const {
		return ignore.size();
	}
protected:
	set<IHandleEntity*> ignore;
};


float Avoid::trace(const Vector& pos, const Vector& goal) const {
	edict_t *selfEnt = ctx.getSelf()->getEdict();
	FilterList filter;
	filter.add(selfEnt).add(ctx.getSelf()->getTarget()).add(BasePlayer(selfEnt).getGroundEntity());
	trace_t tr;
	int ignoreSize = filter.getIgnoreSize();
	do {
		UTIL_TraceLine(pos, goal, MASK_PLAYERSOLID, &filter, &tr);
		if (ignoreSize == filter.getIgnoreSize()) {
			break;
		}
		ignoreSize = filter.getIgnoreSize();
	} while (tr.startsolid);
	if (!tr.DidHit() || (tr.plane.normal.LengthSqr() > 0.0f
			&& tr.plane.normal.z <= nav_slope_limit.GetFloat())) {
		return goal.DistTo(pos);
	}
	return tr.startsolid ? MIN_DIST : Max(MIN_DIST, pos.DistTo(tr.endpos));
}
