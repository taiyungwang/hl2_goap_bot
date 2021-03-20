#pragma once

#include "StepBack.h"

/**
 * Defines a state where a bot has to step left from its intended goal.
 */
class StepLeft: public StepBack {
public:
	StepLeft(MoveStateContext& ctx) :
		StepBack(ctx) {
	}

protected:
	virtual MoveState* buildFailedState(const Vector& currPos) const;

	virtual Vector buildDir(const Vector& currPos) const;
};
