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

static ConVar mybot_stuck_threshold("mybot_stuck_threshold", "0.2f");
static ConVar my_bot_stuck_dur_threshold("mybot_stuck_dur_threshold", "5");

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

bool MoveStateContext::checkStuck() {
	Vector currentPos = blackboard.getSelf()->getCurrentPosition();
	float movedDist = previousPos.DistTo(currentPos);
	previousPos = currentPos;
	bool stuck = movedDist < mybot_stuck_threshold.GetFloat();
	if (!stuck) {
		stuckDur = 0;
	} else if (stuckDur++ > my_bot_stuck_dur_threshold.GetInt()) {
		stuckDur = 0;
		return true;
	}
	return false;
}

const bool MoveStateContext::hasGoal() const {
	return dynamic_cast<Stopped*>(state) == nullptr;
}

bool MoveStateContext::reachedGoal(float targetOffset) {
	const Vector& pos = blackboard.getSelf()->getCurrentPosition();
	Vector goalGround(goal);
	goalGround.z = pos.z;
	if (((goal.z > pos.z && goal.z - pos.z < 20.0f) || pos.z - goal.z < HumanHeight)
			&& goalGround.DistTo(pos)
			< targetOffset + HalfHumanWidth) {
		stuck = false;
		return true;
	}
	return false;
}

const trace_t& MoveStateContext::trace(Vector goal) {
	const Player* self = blackboard.getSelf();
	Vector pos = self->getCurrentPosition();
	MoveTraceFilter filter(*self, blackboard.getTarget());
	edict_t* edict = self->getEdict();
	Vector mins = edict->GetCollideable()->OBBMins(),
			maxs = edict->GetCollideable()->OBBMaxs();
	mins.z += StepHeight;
	extern ConVar mybot_debug;
	UTIL_TraceHull(pos, goal, mins, maxs,
			MASK_SOLID_BRUSHONLY, filter, &traceResult, mybot_debug.GetBool());
	return traceResult;
}
