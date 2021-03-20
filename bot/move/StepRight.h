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

private:
	float distance;

	MoveState* buildFailedState(const Vector& currPos) const;

	Vector buildDir(const Vector& currPos) const;

};
