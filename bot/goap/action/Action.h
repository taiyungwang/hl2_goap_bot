#pragma once

#include <goap/WorldCond.h>

class Blackboard;

/**
 * Defines a GOAP action.
 */
class Action {
public:

	Action(Blackboard& blackboard) :
		blackboard(blackboard) {
		SetDefLessFunc(precond);
	}

	virtual ~Action() {
	}

	virtual bool execute() = 0;

	/**
	 * Called once before action is executed.  This can be used to satisfy
	 * dependencies on subsequent actions.
	 */
	virtual void init() {
	}

	/**
	 * @return True if preconditions for the action are satisfied.
	 */
	virtual bool precondCheck() {
		return true;
	}

	/**
	 * @return True if post conditions for the action are satisfied.
	 */
	virtual bool postCondCheck() {
		return true;
	}

	virtual float getCost() const {
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

protected:
	Blackboard& blackboard;

	WorldState precond;

	GoalState effects;

};
