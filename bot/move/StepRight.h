#pragma once

#include "StepLeft.h"

/**
 * Defines a state where a bot has to step to the right of its intended goal.
 */
class StepRight: public StepLeft {
public:
	StepRight(MoveStateContext& ctx, float distance) :
			StepLeft(ctx), distance(distance) {
	}

	MoveState* move(const Vector& currPos);

private:
	static Vector inverse2D(const Vector& dir);

	float distance;

	Avoid* buildAvoidState(const Vector& currPos) const;

};
