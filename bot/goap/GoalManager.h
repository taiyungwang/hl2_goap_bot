#pragma once

#include "WorldCond.h"
#include <queue>

class Action;
class Planner;
class Bot;

/**
 * Manages a list of actions associated with their respective goals.  The goals are repeatedly executed
 * order of priority.
 */

class GoalManager {
public:
	/**
	 * Builds a list of actions and a list of goals.
	 */
	GoalManager(const WorldState& worldState, Bot *self);

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
	template<typename T, typename... Args>
	T* addAction(float priority, Args&&... args) {
		T* action = new T(self, args...);
		addAction(priority, action);
		return action;
	}

	/**
	 * Starts the planning process.  Assumes that all actions
	 * and goals are already added
	 */
	void execute();

private:
	Planner* planBuilder;

	std::queue<int> plan;

	Bot *self;

	const WorldState& worldState;

	std::vector<Action*> actions;

	struct Goal {
		float priority;
		int action;
	};

	std::vector<Goal> goals;

	int currentGoal = 0;

	void reset();

	void addAction(float priority, Action* action);

	bool getNextGoal();
};
