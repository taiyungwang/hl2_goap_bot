#pragma once

#include "MoveState.h"

/**
 * Defines a state where the bot is stopped therefore having no goals.
 */
class Stopped: public MoveState {
public:
	Stopped(MoveStateContext& ctx) :
			MoveState(ctx) {
	}

	virtual MoveState* move(const Vector& currPos);

};
