#include "StepRight.h"

#include "Stopped.h"
#include "StepBack.h"
#include "Avoid.h"
#include "MoveStateContext.h"

Vector StepRight::inverse2D(const Vector& dir) {
	Vector inv(dir);
	inv.Negate();
	inv.z = -inv.z;
	return inv;
}

MoveState* StepRight::buildFailedState(const Vector& currPos) const {
	return new StepBack(ctx);
}

Vector StepRight::buildDir(const Vector& currPos) const {
	return inverse2D(perpLeft2D(ctx.getGoal(), currPos)) * distance;
}
