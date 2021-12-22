#include <goap/Planner.h>
#include "action/Action.h"
#include <util/SimpleException.h>
#include <utlstring.h>

void Planner::addAction(Action *action) {
	int i = actions.size();
	actions.push_back(action);
	const GoalState &currCond = actions[i]->getEffects();
	if (efxToActions.find(currCond) == efxToActions.end()) {
		efxToActions[currCond] = std::vector<int>();
	}
	efxToActions[currCond].push_back(i);
	createNode();
}

void Planner::startSearch(const GoalState &goal) {
	openSet = std::priority_queue<Node*>();
	start = nullptr;
	for (auto& node: nodes) {
		node.isOpen = node.isClosed = false;
		node.parent = -1;
		node.fScore = node.gScore = INFINITY;
		node.currState.clear();
		node.goalState.clear();
	}
	Node &node = nodes.back();
	auto prop = std::get<0>(goal);
	node.goalState[prop] = std::get<1>(goal);
	node.currState[prop] = worldState.at(prop);
	node.isOpen = true;
	openSet.push(&node);
	node.gScore = 0.0f;
	node.fScore = getHeuristicCost(node);
}

bool Planner::searchStep() {
	Node &current = *(openSet.top());
	openSet.pop();
	GoalState goal;
	bool foundGoal = false;
	for(auto i: current.goalState)
	{
		std::get<0>(goal) = std::get<0>(i);
		std::get<1>(goal) = std::get<1>(i);
		if (std::get<1>(goal) != current.currState.at(std::get<0>(goal))) {
			foundGoal = true;
			break;
		}
	}
	current.isClosed = true;
	current.isOpen = false;
	if (!foundGoal) { // no goals need to be satisfied
		start = &current;
		return true;
	}
	if (efxToActions.end() == efxToActions.find(goal)) {
		// no actions can satisfy this precondition.
		return openSet.empty();
	}
	for (int i: efxToActions.at(goal))
	{
		Node &neighbor = nodes[i];
		if (neighbor.isClosed) {
			continue;
		}
		Action *action = actions[neighbor.id];
		if (!action->precondCheck()) {
			neighbor.isClosed = true;
			continue;
		}
		float tentativeGScore = current.gScore + action->getCost();
		neighbor.currState = current.currState;
		neighbor.goalState = current.goalState;
		WorldProp goalProp = std::get<0>(goal);
		neighbor.currState[goalProp] = std::get<1>(goal);
		WorldState &neighborGoal = neighbor.goalState;
		for (auto j: action->getPrecond())
		{
			WorldProp precondProp = std::get<0>(j);
			if (neighborGoal.find(precondProp) == neighborGoal.end()) {
				neighborGoal[precondProp] = std::get<1>(j);
			} else if (std::get<1>(j) != neighborGoal.at(precondProp)) {
				throw SimpleException(CUtlString("Goal state conflict for prop, ")
						+ static_cast<int>(precondProp));
			}
			neighbor.currState[precondProp] = worldState.at(precondProp);
		}
		if (tentativeGScore >= neighbor.gScore) {
			continue;
		}
		neighbor.parent = current.id;
		neighbor.gScore = tentativeGScore;
		neighbor.fScore = neighbor.gScore + getHeuristicCost(neighbor);
		if (!neighbor.isOpen) {
			// found new node
			neighbor.isOpen = true;
			openSet.push(&neighbor);
		}
	}
	return openSet.size() == 0;
}

void Planner::getPath(std::queue<int> &path) const {
	std::queue<int> plan;
	path = plan;
	if (start == nullptr) {
		return;
	}
	for (int i = start->id; i != nodes.size() - 1; i = nodes[i].parent) {
		if (!actions[nodes[i].id]->onPlanningFinished()) {
			path = std::queue<int>();
			return;
		}
		path.push(nodes[i].id);
	}
}

void Planner::createNode() {
	nodes.emplace_back();
	nodes.back().id = nodes.size() - 1;
}

float Planner::getHeuristicCost(const Node &node) const {
	float cost = 0.0f;
	const auto &goals = node.goalState;
	for(auto i: goals)
	{
		const auto &currState = node.currState;
		WorldProp prop = std::get<0>(i);
		if (currState.find(prop) == currState.end()) {
			throw new SimpleException("Unable to find current state.");
		}
		if (std::get<1>(i) != currState.at(prop)) {
			cost += 1.0f;
		}
	}
	return cost;
}
