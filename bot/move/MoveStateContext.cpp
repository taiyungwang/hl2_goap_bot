#include "MoveStateContext.h"

#include "Stopped.h"
#include <player/Bot.h>
#include <player/Blackboard.h>
#include <player/Button.h>
#include <util/BasePlayer.h>
#include <util/UtilTrace.h>
#include <util/EntityUtils.h>
#include <edict.h>

const float MoveStateContext::TARGET_OFFSET = 8.0f;

MoveStateContext::~MoveStateContext() {
	delete state;
}

void MoveStateContext::stop() {
	type = NAV_MESH_INVALID;
	stuck = false;
	ladderDir = CNavLadder::NUM_LADDER_DIRECTIONS;
	targetOffset = 0.0f;
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
	Vector look = ladderDir != CNavLadder::NUM_LADDER_DIRECTIONS ? ladderEnd: getGoal();
	if (ladderDir != CNavLadder::LADDER_DOWN) {
		look.z += blackboard.getSelf()->getEyesPos().DistTo(pos);
	}
	if (blackboard.getBlocker() != nullptr) {
		Vector mins, maxs, blocker;
		blackboard.getBlocker()->GetCollideable()->WorldSpaceTriggerBounds(&mins, &maxs);
		blocker = (maxs + mins) / 2.0f;
		if (blocker.DistTo(pos) < 130.0f) {
			blackboard.getSelf()->setWantToListen(false);
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

bool MoveStateContext::isAtTarget(const Vector& target, float targetOffset) const {
	Vector pos = blackboard.getSelf()->getCurrentPosition();
	pos.z = target.z;
	return pos.DistTo(target) < targetOffset + TARGET_OFFSET;
}

bool MoveStateContext::reachedGoal(float targetOffset) {
	if (isAtTarget(goal, targetOffset)) {
		stuck = false;
		return true;
	}
	return false;
}

const trace_t& MoveStateContext::trace(const Vector& pos, const Vector& goal, bool crouch) {
	return trace(pos, goal, crouch,
			FilterList().add(blackboard.getSelf()->getEdict()) .add(blackboard.getTarget()));
}

const trace_t& MoveStateContext::trace(const Vector& goal, bool crouch) {
	return trace(blackboard.getSelf()->getCurrentPosition(), goal, crouch);
}

const trace_t& MoveStateContext::trace(const Vector& start, const Vector& goal, bool crouch,
		const ITraceFilter& filter) {
	edict_t* edict = blackboard.getSelf()->getEdict();
	Vector mins = edict->GetCollideable()->OBBMins(),
			maxs = edict->GetCollideable()->OBBMaxs(),
			heading(goal - start);
	if (fabs(heading.x) > fabs(heading.y)) {
		mins.x = maxs.x = 0.0f;
	} else {
		mins.y = maxs.y = 0.0f;
	}
	if (crouch) {
		// magic number from https://developer.valvesoftware.com/wiki/Dimensions#Map_Grid_Units:_quick_reference
		// for some reason the OBBMaxs returns 60
		maxs.z -= 24.0f;
	}
	mins.z += 5.0f;
	extern ConVar mybot_debug;
	UTIL_TraceHull((goal - start).Normalized() * HalfHumanWidth + start,
			goal, mins, maxs, MASK_PLAYERSOLID, filter, &traceResult, mybot_debug.GetBool());
	return traceResult;
}
