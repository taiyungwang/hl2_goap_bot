#pragma once

#include "MoveState.h"

class MoveLadder: public MoveState {
public:
	MoveLadder(MoveStateContext& ctx);

	MoveState* move(const Vector& currPos);

private:
	float distance;
};
