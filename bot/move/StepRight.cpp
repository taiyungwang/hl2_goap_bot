#include "StepRight.h"

#include "Stopped.h"
#include "Jump.h"
#include "Avoid.h"
#include "MoveStateContext.h"

MoveState* StepRight::move(const Vector& currPos) {
	if (arrived(currPos)) {
		return new Avoid(ctx, new Jump(ctx));
	}
	if (checkStuck(currPos)) {
		if (ctx.getType() & NAV_MESH_CROUCH) {
			return new Avoid(ctx, new Jump(ctx));
		}
		return new Jump(ctx);
	}
	Vector dest = perpLeft2D(ctx.getGoal(), currPos);
	dest.x = -dest.x;
	dest.y = -dest.y;
	moveStraight(currPos + dest);
	return nullptr;
}
