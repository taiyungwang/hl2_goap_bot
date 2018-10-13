#pragma once

#include "MoveState.h"

class Jump: public MoveState {
public:
	Jump(MoveStateContext& ctx) :
			MoveState(ctx) {
	}

	MoveState* move(const Vector& currPos);

};
