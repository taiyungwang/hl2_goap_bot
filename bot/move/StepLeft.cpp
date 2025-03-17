#include "StepLeft.h"

#include "StepRight.h"
#include "Avoid.h"
#include "Stopped.h"
#include "Jump.h"
#include "MoveStateContext.h"
#include <player/Bot.h>

MoveState* StepLeft::buildFailedState(const Vector& currPos) const {
	return new StepRight(ctx, GenerationStepSize + startPos.DistTo(currPos));
}

Vector StepLeft::buildDir(const Vector& currPos) const {
	return perpLeft2D(ctx.getGoal(), currPos) * GenerationStepSize;
}

