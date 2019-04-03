#include "StepLeft.h"

#include "StepRight.h"
#include "Avoid.h"
#include "Stopped.h"
#include "Jump.h"
#include "MoveStateContext.h"
#include <player/Blackboard.h>
#include <player/Bot.h>

Vector StepLeft::perpLeft2D(const Vector& end, const Vector& start) {
	Vector dir = end - start;
	Vector perp = dir;
	perp.x = -dir.y;
	perp.y = dir.x;
	return perp;
}

MoveState* StepLeft::buildFailedState(const Vector& currPos) const {
	return new StepRight(ctx, GenerationStepSize + startPos.DistTo(currPos));
}

Vector StepLeft::buildDir(const Vector& currPos) const {
	return perpLeft2D(ctx.getGoal(), currPos) * GenerationStepSize;
}

