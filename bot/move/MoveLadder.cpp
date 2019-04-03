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
	if (ctx.getBlackboard().isOnLadder()) {
		distance = ctx.getLadderEnd().DistTo(currPos);
		if (distance <= (dir == CNavLadder::LADDER_UP ? HumanHeight : StepHeight)
				|| (prevDist >= 0.0f && prevDist - distance <= 0.0f
						&& distance <= HumanHeight + ctx.getTargetOffset())) {
			buttons.tap(IN_USE);
			ctx.setLadderDir(CNavLadder::NUM_LADDER_DIRECTIONS);
			return new Stopped(ctx);
		}
	}
	if (!ctx.getBlackboard().isOnLadder()) {
		buttons.tap(IN_USE);
	} else if (ctx.getBlackboard().getAimAccuracy(ctx.getLadderEnd()) >= 0.7f) {
		buttons.hold(IN_FORWARD);
	}
	return nullptr;
}

