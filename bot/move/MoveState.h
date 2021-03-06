#pragma once

class MoveStateContext;
class Vector;

/**
 * FSM State definition for bot movement.
 */
class MoveState {
public:

	MoveState(MoveStateContext &ctx) :
			ctx(ctx) {
	}

	virtual ~MoveState() {
	}

	virtual MoveState* move(const Vector &currPos) = 0;

protected:
	MoveStateContext &ctx;

	void moveStraight(const Vector &destination) const;
};

