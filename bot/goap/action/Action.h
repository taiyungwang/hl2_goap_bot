#pragma once

#include <goap/WorldCond.h>

class Bot;

/**
 * Defines a GOAP action.
 */
class Action {
public:

	Action(Bot *self): self(self) {
	}

	virtual ~Action() {
	}

	virtual bool execute() = 0;

	/**
	 * Called once before action is executed.  This can be used to satisfy
	 * dependencies on subsequent actions.
	 */
	virtual bool init() {
		return true;
	}

	/**
	 * Invoked during planning on precondition check.  A return of false will cause the planner
	 * to ignore this action as a way to satisfy the given goal.
	 */
	virtual bool precondCheck() {
		return true;
	}
	
	/**
	 * @return True if post conditions for the action are satisfied.
	 */
	virtual bool goalComplete() {
		return true;
	}

	virtual float getCost() {
		return 1.0f;
	}

	const GoalState& getEffects() const {
		return effects;
	}

	const WorldState& getPrecond() const {
		return precond;
	}

	virtual bool isInterruptable() const {
		return false;
	}

	/**
	 * Called when an action is interrupted.
	 */
	virtual void abort() {
	}

	virtual float getChanceToExec() const {
		return 1.0f;
	}

protected:
	Bot *self;

	WorldState precond;

	GoalState effects;
};
