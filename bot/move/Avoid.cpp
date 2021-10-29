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
#include <server_class.h>

using namespace std;

extern IVEngineServer* engine;

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
		if (result.startsolid || dynamic_cast<Stopped*>(nextState) != nullptr) {
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
