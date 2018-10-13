#include "Stopped.h"

#include "MoveStateContext.h"
#include "MoveLadder.h"
#include "Avoid.h"
#include "StepLeft.h"
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <in_buttons.h>

MoveState* Stopped::move(const Vector& currPos) {
	if (!ctx.reachedGoal()) {
		return new Avoid(ctx, new StepLeft(ctx));
	}
	if (ctx.nextGoalIsLadderStart() || ctx.getBlackboard().isOnLadder()) {
		return new MoveLadder(ctx);
	}
	if (ctx.getType() & NAV_MESH_CROUCH) {
		ctx.getBlackboard().getButtons().hold(IN_DUCK);
	}
	return nullptr;
}
