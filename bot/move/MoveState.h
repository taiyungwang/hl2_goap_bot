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
	static Vector perpLeft2D(const Vector& end, const Vector& start);

	static Vector inverse2D(const Vector& dir);

	MoveStateContext &ctx;

	void moveStraight(const Vector &destination) const;
};

