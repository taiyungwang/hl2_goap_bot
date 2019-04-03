#pragma once

#include "MoveState.h"
#include <vector.h>

class Avoid;

/**
 * Defines a state where a bot has to step left from its intended goal.
 */
class StepLeft: public MoveState {
public:
	StepLeft(MoveStateContext& ctx);

	virtual MoveState* move(const Vector& currPos);
protected:

	static Vector perpLeft2D(const Vector& end, const Vector& start);

	Vector startPos;

	bool arrived(const Vector& currPos, float expectedDist) const;

	virtual Avoid* buildAvoidState(const Vector& currPos) const;
};
