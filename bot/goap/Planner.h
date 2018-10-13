#pragma once

#include "WorldCond.h"
#include <utlvector.h>
#include <utlqueue.h>

class Action;
class AStar;
class Blackboard;

class Planner {
public:
	/**
	 * Builds a list of actions and a list of goals.
	 */
	Planner(const WorldState& worldState, Blackboard& blackboard);

	~Planner();

	void resetPlanning(bool force);

	/**
	 * Adds a new action to the planner.
	 * @param priority Priority value of the action.  Expected to
	 * be between 0.0 and 1.0 inclusive. The planner chooses an action to execute
	 * based on highest priority.  Priority goals are never considered.
	 */
	template<typename T>
	void addAction(float priority) {
		addAction(new T(blackboard));
		if (priority <= 0.0f) {
			return;
		}
		goals.AddToTail();
		goals.Tail().action = actions.Count() - 1;
		goals.Tail().priority = priority;
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
		float priority;
		int action;
	};

	CUtlVector<Goal> goals;

	int currentGoal = 0;

	void addAction(Action* action);

	void getNextGoal();
};
