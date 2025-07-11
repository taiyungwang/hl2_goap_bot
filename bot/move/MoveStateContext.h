#pragma once

#include <nav_mesh/nav_ladder.h>
#include <gametrace.h>

class MoveState;
class EntityInstance;
class Bot;
class ITraceFilter;
class CBotCmd;

/**
 * Context for the Bot move finite state machine.
 */
class MoveStateContext {
public:
	// Margin of error when determining goal has been reached.
	static const float TARGET_OFFSET;

	/**
	 * @param pos Current position
	 */
	MoveStateContext(Bot *self): self(self) {
		stop();
	}

	~MoveStateContext();

	void setGoal(const Vector& goal) {
		this->goal = goal;
	}

	/**
	 * Handles movement for the current state.
	 *
	 * @type Current CNavArea type.
	 */
	void move(int type);

	const Vector &getGoal() const {
		return goal;
	}

	void setLadderEnd(const Vector& ladderEnd) {
		this->ladderEnd = ladderEnd;
	}

	const Vector& getLadderEnd() const {
		return ladderEnd;
	}

	int getType() const {
		return type;
	}

	void stop();

	/**
	 * @return True if the bot has a goal.
	 */
	const bool hasGoal() const;

	bool reachedGoal() {
		return reachedGoal(targetOffset);
	}

	bool isAtTarget(const Vector& target, float targetOffset) const;

	bool reachedGoal(float targetOffset);

	void setLadderDir(CNavLadder::LadderDirectionType ladderDir) {
		this->ladderDir = ladderDir;
	}

	bool nextGoalIsLadderStart() const {
		return ladderDir != CNavLadder::NUM_LADDER_DIRECTIONS;
	}

	CNavLadder::LadderDirectionType getLadderDir() const {
		return ladderDir;
	}

	Bot *getSelf() {
		return self;
	}

	void setStuck(bool stuck) {
		this->stuck = stuck;
	}

	bool isStuck() const {
		return stuck;
	}

	void setTargetOffset(float offset) {
		targetOffset = offset;
	}

	float getTargetOffset() const {
		return targetOffset;
	}

	/**
	 * Traces from current position to final goal.  Assumes that final goal
	 * and current position are latest.
	 */
	void traceMove(bool crouch) {
		trace(goal, crouch);
	}

	const trace_t& getTraceResult() const {
		return traceResult;
	}

	const trace_t& trace(const Vector& goal, bool crouch);

	const trace_t& trace(const Vector& start, const Vector& goal, bool crouch);

	const trace_t& trace(const Vector& start, const Vector& goal, bool crouch,
			const ITraceFilter& filter);

private:
	float targetOffset;
	Bot *self;
	bool stuck;
	int type;
	Vector goal, ladderEnd;
	CNavLadder::LadderDirectionType ladderDir;
	MoveState* state = nullptr;
	trace_t traceResult;
};
