#include "MoveLadder.h"

#include "Stopped.h"
#include "MoveStateContext.h"
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
	Bot *self = ctx.getSelf();
	float prevDist = remainingDist;
	if (self->isOnLadder()) {
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
	Buttons& buttons = self->getButtons();
	if (self->isOnLadder()) {
		if (!moved) {
			if (startedClimbing) {
				if (atEnd || remainingDist < TARGET_OFFSET) {
					buttons.tap(IN_USE);
				}
				if (stuckFrames++ > 40) {
					ctx.setLadderDir(CNavLadder::NUM_LADDER_DIRECTIONS);
					return new Stopped(ctx);
				}
			}
		} else {
			stuckFrames = 0;
		}
		if (self->getAimAccuracy() >= 0.9f) {
			startedClimbing = true;
			buttons.hold(IN_FORWARD);
		}
	} else {
		if (startedClimbing && !moved && atEnd) {
			ctx.setLadderDir(CNavLadder::NUM_LADDER_DIRECTIONS);
			return new Stopped(ctx);
		}
		startedClimbing = false;
		buttons.tap(IN_USE);
		moveStraight(ctx.getLadderEnd());
	}
	return nullptr;
}

