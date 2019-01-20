#include "StepLeft.h"

#include "StepRight.h"
#include "Avoid.h"
#include "Stopped.h"
#include "Jump.h"
#include "MoveStateContext.h"
#include <player/Blackboard.h>
#include <player/Bot.h>

StepLeft::StepLeft(MoveStateContext& ctx) :
		MoveState(ctx) {
	startPos = ctx.getBlackboard().getSelf()->getCurrentPosition();
}

MoveState* StepLeft::move(const Vector& currPos) {
	if (arrived(currPos)) {
		return new Avoid(ctx, new Jump(ctx));
	}
	if (checkStuck(currPos)) {
		return new StepRight(ctx);
	}
	moveStraight(perpLeft2D(ctx.getGoal(), currPos) + currPos);
	return nullptr;
}

bool StepLeft::arrived(const Vector& currPos) const {
	return (currPos - startPos).Length() >= GenerationStepSize;
}

Vector StepLeft::perpLeft2D(const Vector& end, const Vector& start) {
	Vector dir = end - start;
	Vector perp = dir;
	perp.x = -dir.y;
	perp.y = dir.x;
	return perp;
}

Vector StepLeft::inverse2D(const Vector& dir) {
	Vector inv(dir);
	inv.Negate();
	inv.z = -inv.z;
	return inv;
}

