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
	auto self = blackboard.getSelf();
	Vector pos = self->getCurrentPosition();
	MoveState* newState = state->move(pos);
	if (newState != nullptr) {
		delete state;
		state = newState;
	}
	// do look
	bool onLadder = blackboard.isOnLadder() || ladderDir != CNavLadder::NUM_LADDER_DIRECTIONS;
	Vector look = goal;
	look.z += blackboard.getSelf()->getEyesPos().DistTo(pos);
	if (blackboard.getBlocker() != nullptr) {
		Vector mins, maxs, blocker;
		blackboard.getBlocker()->GetCollideable()->WorldSpaceTriggerBounds(&mins, &maxs);
		blocker = (maxs + mins) / 2.0f;
		if (blocker.DistTo(pos) < 130.0f) {
			self->setWantToListen(false);
			self->setViewTarget(blocker);
		} else {
			blackboard.setBlocker(nullptr);
		}
	} else if (blackboard.getSelf()->getVision().getTargetedPlayer() == 0 && !onLadder) {
		self->setViewTarget(look);
	}
}

const bool MoveStateContext::hasGoal() const {
	return dynamic_cast<Stopped*>(state) == nullptr;
}

bool MoveStateContext::isAtTarget(const Vector& target, float targetOffset) const {
	Vector pos = blackboard.getSelf()->getCurrentPosition();
	if (std::abs(target.z - pos.z) > JumpHeight) {
		return false;
	}
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

class FilterSelfAndTarget: public CTraceFilter {
public:
	FilterSelfAndTarget(edict_t *self, edict_t *target) :
			self(self->GetIServerEntity()),
			target(target == nullptr ? nullptr : target->GetIServerEntity()) {
	}

	virtual ~FilterSelfAndTarget() {
	}

	bool ShouldHitEntity(IHandleEntity *pHandleEntity,
			int contentsMask) override {
		return pHandleEntity != self && pHandleEntity != target;
	}

private:
	IHandleEntity *self, *target;
};

const trace_t& MoveStateContext::trace(const Vector& pos, const Vector& goal, bool crouch) {
	return trace(pos, goal, crouch,
			FilterSelfAndTarget(blackboard.getSelf()->getEdict(), blackboard.getTarget()));
}

const trace_t& MoveStateContext::trace(const Vector& goal, bool crouch) {
	return trace(blackboard.getSelf()->getCurrentPosition(), goal, crouch);
}

const trace_t& MoveStateContext::trace(const Vector& start, const Vector& goal, bool crouch,
		const ITraceFilter& filter) {
	blackboard.getSelf()->traceMove(traceResult, start, goal, crouch, filter);
	return traceResult;
}
