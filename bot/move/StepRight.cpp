#include "StepRight.h"

#include "Stopped.h"
#include "Jump.h"
#include "Avoid.h"
#include "MoveStateContext.h"

MoveState* StepRight::move(const Vector& currPos) {
	if (checkStuck(currPos, ctx.getGoal())) {
		if (startPos.DistTo(currPos) < ctx.getTargetOffset()) {
			return new Jump(ctx);
		}
		return buildAvoidState(currPos);
	}
	if (arrived(currPos, distance)) {
		return buildAvoidState(currPos);
	}
	moveStraight(currPos + inverse2D(perpLeft2D(ctx.getGoal(), currPos)) * distance);
	return nullptr;
}


Vector StepRight::inverse2D(const Vector& dir) {
	Vector inv(dir);
	inv.Negate();
	inv.z = -inv.z;
	return inv;
}

Avoid* StepRight::buildAvoidState(const Vector& currPos) const {
	return new Avoid(ctx, new Jump(ctx));
}
