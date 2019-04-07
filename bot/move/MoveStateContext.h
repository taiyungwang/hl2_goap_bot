#pragma once

#include <navmesh/nav_ladder.h>
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
	 * Radius of bot
	 */
	static float SELF_RADIUS,
	/**
	 * Margin of error for to consider a position reached.
	 */
	TARGET_OFFSET;

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

	bool isStuck() const {
		return stuck;
	}

	void setStuck(bool stuck) {
		this->stuck = stuck;
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
	void traceMove() {
		trace(goal);
	}

	const trace_t& trace(Vector goal, edict_t* ignore = nullptr);

	const trace_t& getTraceResult() const {
		return traceResult;
	}

private:
	float targetOffset;
	bool stuck;
	Blackboard& blackboard;
	int type;
	Vector goal, ladderEnd;
	CNavLadder::LadderDirectionType ladderDir;
	MoveState* state = nullptr;
	edict_t* blocker = nullptr;
	trace_t traceResult;
};
