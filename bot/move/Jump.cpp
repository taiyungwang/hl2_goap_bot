#include "Jump.h"

#include "MoveStateContext.h"
#include "StepLeft.h"
#include "Avoid.h"
#include <player/Blackboard.h>
#include <player/Buttons.h>

MoveState* Jump::move(const Vector& currPos) {
	Vector pos(currPos), goal((ctx.getGoal() - currPos).Normalized() * 136.0f + pos);
	pos.z += 48.0f;
	goal.z = pos.z;
	if ((!ctx.getBlackboard().isOnLadder() && ctx.trace(pos, goal, true).DidHit())
			|| !ctx.getBlackboard().getButtons().jump()) {
		return new Avoid(ctx, new StepLeft(ctx));
	}
	moveStraight(ctx.getGoal());
	return nullptr;
}
