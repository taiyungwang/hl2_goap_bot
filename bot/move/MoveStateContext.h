#pragma once

#include <nav_mesh/nav_ladder.h>
#include <gametrace.h>

class MoveState;
class EntityInstance;
class Blackboard;
class CBotCmd;

/**
 * Context for the Bot move finite state machine.
 */
class MoveStateContext {
public:

	/**
	 * @param pos Current position
	 */
	MoveStateContext(Blackboard& blackboard) :
			blackboard(blackboard) {
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

	Blackboard& getBlackboard() {
		return blackboard;
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

	const trace_t& trace(const Vector& start, Vector goal, bool crouch);

	const trace_t& trace(Vector goal, bool crouch);

	trace_t& getTraceResult() {
		return traceResult;
	}

private:
	float targetOffset;
	Blackboard& blackboard;
	bool stuck;
	int type;
	Vector goal, ladderEnd;
	CNavLadder::LadderDirectionType ladderDir;
	MoveState* state = nullptr;
	edict_t* blocker = nullptr;
	trace_t traceResult;
};
