#pragma once

#include "WorldCond.h"
#include <utlvector.h>
#include <utlqueue.h>

class Action;
class AStar;
class Blackboard;

/**
 * Manages a list of actions associated with their respective goals.  The goals are repeatedly executed
 * order of priority.
 */

class GoalManager {
public:
	/**
	 * Builds a list of actions and a list of goals.
	 */
	GoalManager(const WorldState& worldState, Blackboard& blackboard);

	~GoalManager();

	void resetPlanning(bool force);

	/**
	 * Adds a new action to the planner.
	 * @param priority Priority value of the action.  Expected to
	 * be between 0.0 and 1.0 inclusive. The planner chooses an action to execute
	 * based on highest priority.  0 Priority goals are never considered.
	 * @param chanceToExec probability this action will be executed if conditions are met.
	 * Exepected values are 0.0 and 1.0 inclusive.
	 */
	template<typename T>
	T* addAction(float priority, float chanceToExec = 1.0f) {
		T* action = new T(blackboard);
		addAction(action);
		if (priority <= 0.0f) {
			return action;
		}
		goals.AddToTail();
		goals.Tail().action = actions.Count() - 1;
		goals.Tail().priority = priority;
		goals.Tail().chanceToExec = chanceToExec;
		return action;
	}

	/**
	 * Starts the planning process.  Assumes that all actions
	 * and goals are already added
	 */
	void execute();

private:
	void reset();

	enum class State {
		ACTION, PLANNING, REPLAN
	} state = State::REPLAN;

	AStar* planBuilder;

	CUtlQueue<int> plan;

	Blackboard& blackboard;

	const WorldState& worldState;

	CUtlVector<Action*> actions;

	struct Goal {
		float priority, chanceToExec;
		int action;
	};

	CUtlVector<Goal> goals;

	int currentGoal = 0;

	void addAction(Action* action);

	void getNextGoal();
};
