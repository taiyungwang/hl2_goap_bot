#include "MoveLadder.h"

#include "Stopped.h"
#include "MoveStateContext.h"
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <player/Bot.h>
#include <eiface.h>
#include <in_buttons.h>

MoveLadder::MoveLadder(MoveStateContext& ctx) :
		MoveState(ctx) {
	distance = -1.0f;
}

MoveState* MoveLadder::move(const Vector& currPos) {
	float prevDist = distance;
	CNavLadder::LadderDirectionType dir = ctx.getLadderDir();
	Buttons& buttons = ctx.getBlackboard().getButtons();
	bool onLadder = ctx.getBlackboard().isOnLadder();
	if (onLadder && !startedClimbing) {
		startedClimbing = true;
	}
	if (startedClimbing) {
		distance = ctx.getLadderEnd().DistTo(currPos);
		if (prevDist >= 0.0f && prevDist - distance <= 0.0f) {
			if (onLadder) {
				buttons.tap(IN_USE);
			}
			ctx.setLadderDir(CNavLadder::NUM_LADDER_DIRECTIONS);
			if (!distance <= (dir == CNavLadder::LADDER_UP ? 3.0f * HumanHeight : StepHeight)
					+ MoveStateContext::TARGET_OFFSET) {
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

