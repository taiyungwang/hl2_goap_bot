#pragma once

#include "MoveState.h"
#include <vector.h>

/**
 * Defines a state where a bot has to step left from its intended goal.
 */
class StepLeft: public MoveState {
public:
	StepLeft(MoveStateContext& ctx);

	virtual MoveState* move(const Vector& currPos);
protected:

	static Vector perpLeft2D(const Vector& end, const Vector& start);

	static Vector inverse2D(const Vector& dir);

	Vector startPos;

	bool arrived(const Vector& currPos) const;

};
