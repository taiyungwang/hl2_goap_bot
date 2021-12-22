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
	 * Invoked during planning on precondition check.  A return of false will cause the planner
	 * to ignore this action as a way to satisfy the given goal.
	 */
	virtual bool precondCheck() {
		return true;
	}

	/**
	 * Invoked when planning is complete.  The call to this callback is ordered based on
	 * order of action.  Returning false will cause the plan to fail.
	 */
	virtual bool onPlanningFinished() {
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

	float getChanceToExec() const {
		return chanceToExec;
	}

protected:
	Blackboard& blackboard;

	WorldState precond;

	GoalState effects;

	float chanceToExec = 1.0f;
};
