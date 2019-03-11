#include "MoveStateContext.h"

#include "Stopped.h"
#include <player/Bot.h>
#include <player/Blackboard.h>
#include <player/Button.h>
#include <edict.h>

const float MoveStateContext::TARGET_OFFSET = GenerationStepSize / 2.0f;

MoveStateContext::~MoveStateContext() {
	delete state;
}

MoveStateContext::MoveStateContext(Blackboard& blackboard) :
		blackboard(blackboard) {
	stuck = false;
	type = NAV_MESH_INVALID;
	state = new Stopped(*this);
	ladderDir = CNavLadder::NUM_LADDER_DIRECTIONS;
}

void MoveStateContext::stop() {
	delete state;
	stuck = false;
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
	if ((goal - blackboard.getSelf()->getCurrentPosition()).AsVector2D().Length()
			<= TARGET_OFFSET || blackboard.isOnLadder()) {
		stuck = false;
		return true;
	}
	return false;
}
