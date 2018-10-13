#include "Jump.h"

#include "MoveStateContext.h"
#include "Stopped.h"
#include "Avoid.h"
#include "StepLeft.h"
#include <player/Blackboard.h>
#include <player/Buttons.h>

MoveState* Jump::move(const Vector& currPos) {
	if (!ctx.getBlackboard().getButtons().jump()) {
		if (checkStuck(currPos)) {
			return new Stopped(ctx);
		}
		return new Avoid(ctx, new Stopped(ctx));
	}
	moveStraight(ctx.getGoal());
	return nullptr;
}
