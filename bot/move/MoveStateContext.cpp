#include "MoveStateContext.h"

#include "Stopped.h"
#include "MoveTraceFilter.h"
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
	stuck = false;
	ladderDir = CNavLadder::NUM_LADDER_DIRECTIONS;
	targetOffset = 0.0f;
	if (state != nullptr) {
		delete state;
	}
	previousPos = blackboard.getSelf()->getCurrentPosition();
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
	const Vector& pos = blackboard.getSelf()->getCurrentPosition();
	return ((target.z > pos.z && target.z - pos.z < 20.0f) || pos.z - target.z < HumanHeight)
			&& Vector(target.x, target.y, pos.z).DistTo(pos) < targetOffset + HalfHumanWidth;
}

bool MoveStateContext::reachedGoal(float targetOffset) {
	if (isAtTarget(goal, targetOffset)) {
		stuck = false;
		return true;
	}
	return false;
}

const trace_t& MoveStateContext::trace(const Vector& pos, Vector goal, bool crouch) {
	const Player* self = blackboard.getSelf();
	MoveTraceFilter filter(*self, blackboard.getTarget());
	edict_t* edict = self->getEdict();
	Vector mins = edict->GetCollideable()->OBBMins(),
			maxs = edict->GetCollideable()->OBBMaxs();
	if (crouch) {
		// magic number from https://developer.valvesoftware.com/wiki/Dimensions#Map_Grid_Units:_quick_reference
		// for some reason the OBBMaxs returns 60
		maxs.z -= 24.0f;
	}
	mins.z += StepHeight;
	extern ConVar mybot_debug;
	UTIL_TraceHull(pos, goal, mins, maxs,
			MASK_SOLID_BRUSHONLY, filter, &traceResult, mybot_debug.GetBool());
	return traceResult;
}

const trace_t& MoveStateContext::trace(Vector goal, bool crouch) {
	return trace(blackboard.getSelf()->getCurrentPosition(), goal, crouch);
}
