#include "MoveStateContext.h"

#include "Stopped.h"
#include <player/Bot.h>
#include <player/Blackboard.h>
#include <util/UtilTrace.h>

const float MoveStateContext::TARGET_OFFSET = 9.0f;

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
}

const bool MoveStateContext::hasGoal() const {
	return dynamic_cast<Stopped*>(state) == nullptr;
}

bool MoveStateContext::isAtTarget(const Vector& target, float targetOffset) const {
	Vector pos = blackboard.getSelf()->getCurrentPosition();
	if (std::abs(target.z - pos.z) > JumpCrouchHeight) {
		return false;
	}
	pos.z = target.z;
	return pos.DistTo(target) <= targetOffset + TARGET_OFFSET;
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
