#pragma once

#include "MoveState.h"

class MoveLadder: public MoveState {
public:
	/**
	 * Margin of error for to consider a position reached.
	 */
	static float TARGET_OFFSET;

	MoveLadder(MoveStateContext& ctx);

	MoveState* move(const Vector& currPos);

private:
	float remainingDist;

	int stuckFrames = 0;

	bool startedClimbing = false;
};
