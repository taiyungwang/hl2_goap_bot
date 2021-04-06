#pragma once

#include "WorldCond.h"
#include <utlmap.h>
#include <utlpriorityqueue.h>
#include <utlvector.h>
#include <utlstack.h>
#include <utlqueue.h>

class Action;

/**
 * Builds a list of actions for a plan with a given goal using A * search.
 */
class Planner {
public:
	/**
	 * Builds a map of effects to available actions.
	 *
	 * @param actions List of available actions.
	 * @param The current world state.
	 */
	Planner(const WorldState& worldState);

	void addAction(Action* action);

	/**
	 * Resets and initializes a new search.
	 */
	void startSearch(const GoalState& goal);

	/**
	 * Executes one iteration of the search for a given node.
	 *
	 * @return True if search is done.
	 */
	bool searchStep();

	/**
	 * Gets the path of the search result.
	 */
	void getPath(CUtlQueue<int>& path) const;

private:
	struct Node {
		bool isOpen, isClosed;
		int id, parent;
		WorldState goalState, currState;
		float gScore, fScore;
	};

	static void copy(WorldState& left, const WorldState& right);

	CUtlVector<Action*> actions;

	const WorldState& worldState;

	/**
	 * Map of effects to actions that satisfy the effect.
	 */
	CUtlMap<GoalState, CCopyableUtlVector<int>> efxToActions;

	CUtlVector<Node> nodes;

	CUtlPriorityQueue<Node*> openSet;

	const Node* start = nullptr;

	void createNode();

	float getHeuristicCost(const Node& node) const;
};
