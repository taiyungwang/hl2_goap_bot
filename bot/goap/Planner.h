#pragma once

#include "WorldCond.h"
#include <unordered_map>
#include <queue>
#include <vector>

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
	Planner(const WorldState &worldState) :
			worldState(worldState) {
		createNode();
	}

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
	void getPath(std::queue<int>& path) const;

private:
	struct Node {
		bool isOpen, isClosed;
		int id, parent;
		WorldState goalState, currState;
		float gScore, fScore;

		bool operator<(const Node &other) const {
			return fScore > other.fScore;
		}
	};

	std::vector<Action*> actions;

	const WorldState& worldState;

	struct GoalStateHash: public std::unary_function<GoalState, std::size_t> {
		std::size_t operator()(const GoalState &k) const {
			return static_cast<std::size_t>(std::get<0>(k))
					^ static_cast<std::size_t>(std::get<1>(k));
		}
	};

	/**
	 * Map of effects to actions that satisfy the effect.
	 */
	std::unordered_map<GoalState, std::vector<int>, GoalStateHash> efxToActions;

	std::vector<Node> nodes;

	std::priority_queue<Node*> openSet;

	const Node* start = nullptr;

	void createNode();

	float getHeuristicCost(const Node& node) const;
};
