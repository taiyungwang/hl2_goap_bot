#include "StepRight.h"

#include "Stopped.h"
#include "StepBack.h"
#include "Avoid.h"
#include "MoveStateContext.h"

MoveState* StepRight::buildFailedState(const Vector& currPos) const {
	return new StepBack(ctx);
}

Vector StepRight::buildDir(const Vector& currPos) const {
	return inverse2D(perpLeft2D(ctx.getGoal(), currPos)) * distance;
}
