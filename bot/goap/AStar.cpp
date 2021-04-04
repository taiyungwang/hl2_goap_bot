#include "AStar.h"

#include "action/Action.h"
#include <util/SimpleException.h>
#include <utlstring.h>

template<typename T, typename U>
bool operator==(const CUtlMap<T, U> &m1, const CUtlMap<T, U> &m2) {
	if (&m1 == &m2) {
		return true;
	}
	if (m1.Count() < m2.Count()) {
		return false;
	}
	FOR_EACH_MAP_FAST(m1, i)
	{
		const auto &m2Idx = m2.Find(m1.Key(i));
		if (!m2.IsValidIndex(m2Idx) || m1[i] != m2[m2Idx]) {
			return false;
		}
	}
	return true;
}

AStar::AStar(const WorldState &worldState) :
		worldState(worldState) {
	efxToActions.SetLessFunc(
			[](const GoalState &g1, const GoalState &g2) {
				return (g1.m_key == g2.m_key && g1.m_value < g2.m_value)
						|| g1.m_key < g2.m_key;
			});
	openSet.SetLessFunc([](Node *const&n1, Node *const&n2) {
		return n1->fScore > n2->fScore;
	});
	createNode();
}

void AStar::addAction(Action *action) {
	int i = actions.Count();
	actions.AddToTail(action);
	const GoalState &currCond = actions[i]->getEffects();
	auto j = efxToActions.Find(currCond);
	if (!efxToActions.IsValidIndex(j)) {
		j = efxToActions.Insert(currCond, CCopyableUtlVector<int>());
	}
	efxToActions[j].AddToTail(i);
	createNode();
}

void AStar::startSearch(const GoalState &goal) {
	openSet.RemoveAll();
	start = nullptr;
	FOR_EACH_VEC(nodes, i)
	{
		nodes[i].isOpen = nodes[i].isClosed = false;
		nodes[i].parent = -1;
		nodes[i].fScore = nodes[i].gScore = INFINITY;
		nodes[i].currState.RemoveAll();
		nodes[i].goalState.RemoveAll();
	}
	Node &node = nodes.Tail();
	node.goalState.Insert(goal.m_key, goal.m_value);
	node.currState.Insert(goal.m_key, worldState[worldState.Find(goal.m_key)]);
	node.isOpen = true;
	openSet.Insert(&node);
	node.gScore = 0.0f;
	node.fScore = getHeuristicCost(node);
}

bool AStar::searchStep() {
	Node &current = *(openSet.ElementAtHead());
	openSet.RemoveAtHead();
	GoalState goal;
	bool foundGoal = false;
	FOR_EACH_MAP_FAST(current.goalState, i)
	{
		goal.m_key = current.goalState.Key(i);
		goal.m_value = current.goalState[i];
		if (goal.m_value
				!= current.currState[current.currState.Find(goal.m_key)]) {
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
	const auto &efxIdx = efxToActions.Find(goal);
	if (!efxToActions.IsValidIndex(efxIdx)) {
		// no actions can satisfy this precondition.
		return openSet.Count() == 0;
	}
	auto &neighbors = efxToActions[efxIdx];
	FOR_EACH_VEC(neighbors, i)
	{
		Node &neighbor = nodes[neighbors[i]];
		if (neighbor.isClosed) {
			continue;
		}
		Action *action = actions[neighbor.id];
		if (!action->precondCheck()) {
			neighbor.isClosed = true;
			continue;
		}
		float tentativeGScore = current.gScore + action->getCost();
		copy(neighbor.currState, current.currState);
		copy(neighbor.goalState, current.goalState);
		WorldState &currState = neighbor.currState;
		currState[currState.Find(goal.m_key)] = goal.m_value;
		const WorldState &precond = action->getPrecond();
		WorldState &goalState = neighbor.goalState;
		bool goalStateConflict = false;
		FOR_EACH_MAP_FAST(precond, j)
		{
			WorldProp prop = precond.Key(j);
			auto k = goalState.Find(prop);
			if (!goalState.IsValidIndex(k)) {
				goalState.Insert(prop, precond[j]);
				currState.Insert(prop, worldState[worldState.Find(prop)]);
			} else {
				if (precond[j] != goalState[k]) {
					throw SimpleException(CUtlString("Goal state conflict for prop, ")
									+ static_cast<int>(prop));
				}
				currState[k] = worldState[worldState.Find(prop)];
			}
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
			openSet.Insert(&neighbor);
		}
	}
	return openSet.Count() == 0;
}

void AStar::getPath(CUtlQueue<int> &path) const {
	path.RemoveAll();
	if (start == nullptr) {
		return;
	}
	for (int i = start->id; i != nodes.Count() - 1; i = nodes[i].parent) {
		if (!actions[nodes[i].id]->onPlanningFinished()) {
			path.RemoveAll();
			return;
		}
		path.Insert(nodes[i].id);
	}
}

void AStar::copy(WorldState &left, const WorldState &right) {
	if (&left == &right) {
		return;
	}
	left.RemoveAll();
	FOR_EACH_MAP_FAST(right, i)
	{
		left.Insert(right.Key(i), right[i]);
	}
}

void AStar::createNode() {
	nodes.AddToTail();
	SetDefLessFunc(nodes.Tail().goalState);
	SetDefLessFunc(nodes.Tail().currState);
	nodes.Tail().id = nodes.Count() - 1;
}

float AStar::getHeuristicCost(const Node &node) const {
	float cost = 0.0f;
	const auto &goals = node.goalState;
	FOR_EACH_MAP_FAST(goals, i)
	{
		WorldProp prop = goals.Key(i);
		const auto &currState = node.currState;
		const auto &j = currState.Find(prop);
		if (!currState.IsValidIndex(j)) {
			throw new SimpleException("Unable to find current state.");
		}
		if (goals[i] != currState[j]) {
			cost += 1.0f;
		}
	}
	return cost;
}
