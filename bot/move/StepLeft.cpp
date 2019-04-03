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
	if (checkStuck(currPos, startPos)) {
		if (startPos.DistTo(currPos) < ctx.getTargetOffset()) {
			return new StepRight(ctx, 2.0f * GenerationStepSize);
		}
		return buildAvoidState(currPos);
	}
	if (arrived(currPos, GenerationStepSize)) {
		return buildAvoidState(currPos);
	}
	moveStraight(perpLeft2D(ctx.getGoal(), currPos) * GenerationStepSize + currPos);
	return nullptr;
}

bool StepLeft::arrived(const Vector& currPos, float expectedDist) const {
	return (currPos - startPos).Length() >= expectedDist;
}

Vector StepLeft::perpLeft2D(const Vector& end, const Vector& start) {
	Vector dir = end - start;
	Vector perp = dir;
	perp.x = -dir.y;
	perp.y = dir.x;
	return perp;
}

Avoid* StepLeft::buildAvoidState(const Vector& currPos) const {
	return new Avoid(ctx, new StepRight(ctx, GenerationStepSize + startPos.DistTo(currPos)));
}

