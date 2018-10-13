#pragma once

#include "StepLeft.h"

/**
 * Defines a state where a bot has to step to the right of its intended goal.
 */
class StepRight: public StepLeft {
public:
	StepRight(MoveStateContext& ctx) :
			StepLeft(ctx) {
	}

	MoveState* move(const Vector& currPos);
};
