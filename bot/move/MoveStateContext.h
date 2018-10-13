#pragma once

#include <navmesh/nav_ladder.h>

class MoveState;
class EntityInstance;
class Blackboard;
class CBotCmd;

/**
 * Context for the Bot move finite state machine.
 */
class MoveStateContext {
public:
	static const float TARGET_OFFSET;

	/**
	 * @param pos Current position
	 */
	MoveStateContext(Blackboard& blackboard);

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

	bool reachedGoal();

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

private:

	bool stuck;
	Blackboard& blackboard;
	int type;
	Vector goal, ladderEnd;
	CNavLadder::LadderDirectionType ladderDir;
	MoveState* state;
};
