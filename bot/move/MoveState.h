#pragma once

#include <vector.h>
class MoveStateContext;

/**
 * FSM State definition for bot movement.
 */
class MoveState {
public:

	MoveState(MoveStateContext& ctx);

	virtual ~MoveState() {
	}

	virtual MoveState* move(const Vector& currPos) = 0;

protected:

	MoveStateContext& ctx;

	bool checkStuck(const Vector& currentPos);

	void moveStraight(const Vector& destination) const;

private:

	Vector prevPos;
	unsigned int moveDur;
};

