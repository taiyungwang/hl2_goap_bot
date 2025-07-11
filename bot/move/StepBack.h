#pragma once

#include "MoveState.h"
#include <vector.h>

class Avoid;

/**
 * Defines the state when a bot has to step back from its original goal.
 */
class StepBack: public MoveState {
public:
	StepBack(MoveStateContext& ctx);

	MoveState* move(const Vector& currPos);

protected:
	Vector startPos;

	[[nodiscard]] bool arrived(const Vector& currPos, float expectedDist) const;

	[[nodiscard]] Avoid* buildAvoidState(const Vector& currPos) const;

	/**
	 * Defines the state to transition to if the current move failed.
	 */
	[[nodiscard]] virtual MoveState* buildFailedState(const Vector& currPos) const;

	/**
	 * Builds the direction of the move.
	 */
	[[nodiscard]] virtual Vector buildDir(const Vector& currPos) const;
};
