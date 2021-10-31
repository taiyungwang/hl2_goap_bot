#pragma once

#include "MoveState.h"

class CGameTrace;
struct edict_t;

/**
 * Defines the state where a bot is moving toward a point but expects obstacles.
 */
class Avoid: public MoveState {
public:
	Avoid(MoveStateContext& ctx, MoveState* nextState) :
			MoveState(ctx), nextState(nextState), blocker(nullptr) {
	}
	/**
	 * Moves toward the given goal, but attempt to avoid any local obstacles.
	 * Transitions to Straight if no obstacle ahead.  Transitions to StepLeft
	 * if the bot is stuck.
	 */
	virtual MoveState* move(const Vector& currPos);

private:
	static void setTeamWall(edict_t* blocker, int team);

	edict_t* blocker;

	MoveState* nextState;

	float trace(const Vector& pos, const Vector& goal) const;
};
