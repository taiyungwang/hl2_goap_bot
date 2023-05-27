#include "Stopped.h"

#include "MoveStateContext.h"
#include "MoveLadder.h"
#include "Avoid.h"
#include "StepBack.h"
#include "Jump.h"
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <player/Buttons.h>
#include <in_buttons.h>

MoveState* Stopped::move(const Vector& currPos) {
	if (ctx.getBlackboard().getSelf()->isOnLadder()) {
		float delta = ctx.getLadderEnd().z - currPos.z;
		if (delta > HumanHeight + ctx.getTargetOffset() || delta < 0.0f) {
			// bot is not at the end of the ladder.
			return new Jump(ctx);
		}
		if (ctx.nextGoalIsLadderStart()) {
			return new MoveLadder(ctx);
		}
	}
	if (!ctx.reachedGoal()) {
		return new Avoid(ctx, new Jump(ctx));
	}
	if (ctx.getType() & NAV_MESH_CROUCH) {
		ctx.getBlackboard().getButtons().hold(IN_DUCK);
	}
	return nullptr;
}
