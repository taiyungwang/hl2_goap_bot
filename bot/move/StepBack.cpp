#include "StepBack.h"

#include "MoveStateContext.h"
#include "Avoid.h"
#include "Stopped.h"
#include "player/Bot.h"

StepBack::StepBack(MoveStateContext& ctx): MoveState(ctx) {
	startPos = ctx.getSelf()->getCurrentPosition();
}

MoveState* StepBack::move(const Vector& currPos) {
	if (ctx.isStuck()) {
		ctx.setStuck(false);
		return startPos.DistTo(currPos) < ctx.getTargetOffset() ?
			buildFailedState(currPos) : buildAvoidState(currPos);
	}
	if (arrived(currPos, GenerationStepSize)) {
		return buildAvoidState(currPos);
	}
	moveStraight(buildDir(currPos) + currPos);
	return nullptr;
}

bool StepBack::arrived(const Vector& currPos, float expectedDist) const {
	return (currPos - startPos).Length() >= expectedDist;
}

Avoid* StepBack::buildAvoidState(const Vector& currPos) const {
	return new Avoid(ctx, buildFailedState(currPos));
}

MoveState* StepBack::buildFailedState(const Vector& currPos) const {
	return new Stopped(ctx);
}

Vector StepBack::buildDir(const Vector& currPos) const {
	return (currPos - ctx.getGoal()).Normalized() * GenerationStepSize;
}
