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
	auto &bb = ctx.getBlackboard();
	auto self = bb.getSelf();
	float prevDist = remainingDist;
	if (bb.isOnLadder()) {
		remainingDist = std::abs(currPos.z - ctx.getLadderEnd().z);
		if (prevDist < 0.0f) {
			prevDist = remainingDist;
		}
	}
	bool climbingUp = ctx.getLadderDir() == CNavLadder::LADDER_UP,
			moved = prevDist - remainingDist > 0.0f,
			atEnd = (climbingUp && ctx.getLadderEnd().z < currPos.z)
			|| (!climbingUp && currPos.z - ctx.getLadderEnd().z <= StepHeight);
	Vector lookAt = ctx.getLadderEnd();
	if (climbingUp) {
		lookAt.z += 3.0f * HumanHeight;
	}
	self->setViewTarget(lookAt);
	Buttons& buttons = bb.getButtons();
	if (bb.isOnLadder()) {
		if (!moved) {
			if (startedClimbing && (atEnd || remainingDist < TARGET_OFFSET)) {
				buttons.tap(IN_USE);
				if (stuckFrames++ > 40) {
					ctx.setLadderDir(CNavLadder::NUM_LADDER_DIRECTIONS);
					return new Stopped(ctx);
				}
			}
		} else {
			startedClimbing = true;
			stuckFrames = 0;
		}
		if (self->getAimAccuracy() >= 0.9f) {
			buttons.hold(IN_FORWARD);
		}
	} else {
		if (startedClimbing && !moved && atEnd) {
			ctx.setLadderDir(CNavLadder::NUM_LADDER_DIRECTIONS);
			return new Stopped(ctx);
		}
		startedClimbing = false;
		self->lookStraight();
		buttons.tap(IN_USE);
		moveStraight(ctx.getLadderEnd());
	}
	return nullptr;
}

