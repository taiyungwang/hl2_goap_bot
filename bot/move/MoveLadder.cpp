#include "MoveLadder.h"

#include "Stopped.h"
#include "MoveStateContext.h"
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <player/Bot.h>
#include <eiface.h>
#include <iplayerinfo.h>
#include <in_buttons.h>

MoveLadder::MoveLadder(MoveStateContext& ctx) :
		MoveState(ctx) {
	distance = HUGE_VALF;
	shouldBeOnLadder = ctx.getBlackboard().isOnLadder();
	ctx.setGoal(ctx.getLadderEnd());
}

MoveState* MoveLadder::move(const Vector& currPos) {
	extern ConVar my_bot_target_offset;
	float lastDistance = distance;
	distance = fabs(ctx.getGoal().z - currPos.z);
	Blackboard& blackboard = ctx.getBlackboard();
	Buttons& buttons = blackboard.getButtons();
	if (blackboard.getAimAccuracy(ctx.getGoal()) > 0.7f
			|| (ctx.getLadderDir() == CNavLadder::LADDER_UP
					&& blackboard.getSelf()->getEyesPos().z + 15.0f > ctx.getGoal().z)
					|| (ctx.getLadderDir() == CNavLadder::LADDER_DOWN && distance < JumpHeight)) {
		buttons.hold(IN_FORWARD);
	}
	// if we are stuck, check to see if the ladder end is near the head if we are climbing up.
	if (distance <= MoveStateContext::TARGET_OFFSET
			|| (distance >= lastDistance
					&& ctx.getLadderDir() == CNavLadder::LADDER_UP
					&& fabs(distance - HumanHeight) <= 10.0f)) {
		if (blackboard.isOnLadder()) {
			// if on ladder, get off
			buttons.tap(IN_USE);
			return nullptr;
		}
		ctx.setLadderDir(CNavLadder::NUM_LADDER_DIRECTIONS);
		return new Stopped(ctx);
	}
	// if not on ladder, get on
	if (!blackboard.isOnLadder()) {
		if (shouldBeOnLadder) {
			ctx.setStuck(true);
			return new Stopped(ctx);
		}
		shouldBeOnLadder = true;
		buttons.tap(IN_USE);
		return nullptr;
	}
	return nullptr;
}

