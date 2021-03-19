#include "MoveLadder.h"

#include "Stopped.h"
#include "MoveStateContext.h"
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <player/Bot.h>
#include <eiface.h>
#include <in_buttons.h>

float MoveLadder::TARGET_OFFSET = 5.0f;

MoveLadder::MoveLadder(MoveStateContext& ctx) :
		MoveState(ctx) {
	remainingDist = -1.0f;
}

MoveState* MoveLadder::move(const Vector& currPos) {
	float prevDist = remainingDist;
	CNavLadder::LadderDirectionType dir = ctx.getLadderDir();
	Buttons& buttons = ctx.getBlackboard().getButtons();
	bool onLadder = ctx.getBlackboard().isOnLadder();
	if (onLadder && !startedClimbing) {
		startedClimbing = true;
	}
	if (startedClimbing) {
		remainingDist = ctx.getLadderEnd().DistTo(currPos);
		if (prevDist >= 0.0f && prevDist - remainingDist <= 0.0f) {
			// we didn't move this frame.
			if (onLadder) {
				buttons.tap(IN_USE);
			}
			ctx.setLadderDir(CNavLadder::NUM_LADDER_DIRECTIONS);
			if (remainingDist > (dir == CNavLadder::LADDER_UP ? 3.0f * HumanHeight : StepHeight)
					+ TARGET_OFFSET) {
				ctx.setStuck(true);
			}
			return new Stopped(ctx);
		}
	} else if (!ctx.getBlackboard().isOnLadder()) {
		buttons.tap(IN_USE);
	}
	if (ctx.getBlackboard().getAimAccuracy(ctx.getLadderEnd()) >= 0.7f) {
		buttons.hold(IN_FORWARD);
		moveStraight(ctx.getLadderEnd());
	}
	return nullptr;
}

