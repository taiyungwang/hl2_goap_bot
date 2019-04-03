#include "MoveStateContext.h"

#include "Stopped.h"
#include <player/Bot.h>
#include <player/Blackboard.h>
#include <player/Button.h>
#include <util/BasePlayer.h>
#include <util/UtilTrace.h>
#include <util/EntityUtils.h>
#include <edict.h>

MoveStateContext::~MoveStateContext() {
	delete state;
}

void MoveStateContext::stop() {
	type = NAV_MESH_INVALID;
	ladderDir = CNavLadder::NUM_LADDER_DIRECTIONS;
	stuck = false;
	targetOffset = 5.0f;
	if (state != nullptr) {
		delete state;
	}
	state = new Stopped(*this);
}

void MoveStateContext::move(int type) {
	this->type = type;
	Vector pos = blackboard.getSelf()->getCurrentPosition();
	MoveState* newState = state->move(pos);
	if (newState != nullptr) {
		delete state;
		state = newState;
	}
	// do look
	Vector look = blackboard.isOnLadder() ? ladderEnd: getGoal();
	if (ladderDir != CNavLadder::LADDER_DOWN) {
		look.z += blackboard.getSelf()->getEyesPos().DistTo(pos);
	}
	if (blackboard.getBlocker() != nullptr) {
		Vector mins, maxs, blocker;
		blackboard.getBlocker()->GetCollideable()->WorldSpaceTriggerBounds(&mins, &maxs);
		blocker = (maxs + mins) / 2.0f;
		if (blocker.DistTo(pos) < 130.0f) {
			blackboard.setViewTarget(blocker);
		} else {
			blackboard.setBlocker(nullptr);
		}
	} else if (blackboard.getTargetedPlayer() == nullptr || nextGoalIsLadderStart()
			|| blackboard.isOnLadder()) {
		blackboard.setViewTarget(look);
	}
}

const bool MoveStateContext::hasGoal() const {
	return dynamic_cast<Stopped*>(state) == nullptr;
}

bool MoveStateContext::reachedGoal() {
	if (goal.AsVector2D().DistTo(blackboard.getSelf()->getCurrentPosition().AsVector2D())
			<= targetOffset) {
		stuck = false;
		return true;
	}
	return false;
}

const trace_t& MoveStateContext::trace(Vector goal, edict_t* ignore) {
	const Player* self = blackboard.getSelf();
	Vector pos = self->getCurrentPosition();
	static const float HALF_WIDTH = 16.0f;
	static const float FOREHEAD = HumanHeight - HumanEyeHeight;
	extern ConVar mybot_debug;
	FilterList filter;
	filter.add(self->getEdict()).add(BasePlayer(self->getEdict()).getGroundEntity())
					.add(blackboard.getTarget()).add(ignore);
	Vector mins(-HALF_WIDTH, -HALF_WIDTH, 0);
	UTIL_TraceHull(pos, goal, mins,
			Vector(HALF_WIDTH, HALF_WIDTH, self->getEyesPos().z - pos.z + FOREHEAD),
			MASK_SOLID_BRUSHONLY, filter, &traceResult, mybot_debug.GetBool());
	if (traceResult.DidHit() && !traceResult.allsolid &&
			FClassnameIs(reinterpret_cast<IServerEntity*>(traceResult.m_pEnt)->GetNetworkable()->GetEdict()
					, "worldspawn")) {
		// check to see if worldspawn hit is below StepHeight (18.0f);
		pos.z += StepHeight;
		goal.z += StepHeight;
		UTIL_TraceHull(pos, goal, mins,
				Vector(HALF_WIDTH, HALF_WIDTH, self->getEyesPos().z - pos.z + FOREHEAD),
				MASK_SOLID_BRUSHONLY, filter, &traceResult, mybot_debug.GetBool());
	}
	return traceResult;
}
