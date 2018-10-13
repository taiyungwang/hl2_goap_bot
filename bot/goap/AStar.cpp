#include "AStar.h"

#include "action/Action.h"
#include <util/SimpleException.h>

template<typename T, typename U>
bool operator==(const CUtlMap<T, U>& m1, const CUtlMap<T, U>& m2) {
	if (&m1 == &m2) {
		return true;
	}
	if (m1.Count() < m2.Count()) {
		return false;
	}
	FOR_EACH_MAP_FAST(m1, i) {
		const auto& m2Idx = m2.Find(m1.Key(i));
		if (!m2.IsValidIndex(m2Idx) || m1[i] != m2[m2Idx]) {
			return false;
		}
	}
	return true;
}

AStar::AStar(const WorldState& worldState) :
		worldState(worldState) {
	efxToActions.SetLessFunc([] (const GoalState& g1, const GoalState& g2) {
		return (g1.m_key == g2.m_key && g1.m_value < g2.m_value)
		|| g1.m_key < g2.m_key;
	});
	openSet.SetLessFunc([] (Node * const & n1,
			Node * const & n2) {
		return n1->fScore > n2->fScore;
	});
	createNode();
}

void AStar::addAction(Action* action) {
	int i = actions.Count();
	actions.AddToTail(action);
	const GoalState& currCond = actions[i]->getEffects();
	auto j = efxToActions.Find(currCond);
	if (!efxToActions.IsValidIndex(j)) {
		j = efxToActions.Insert(currCond, CCopyableUtlVector<int>());
	}
	efxToActions[j].AddToTail(i);
	createNode();
}

void AStar::startSearch(const GoalState& goal) {
	openSet.RemoveAll();
	start.RemoveAll();
	FOR_EACH_VEC(nodes, i)
	{
		nodes[i].isOpen = nodes[i].isClosed = false;
		nodes[i].parent = -1;
		nodes[i].fScore = nodes[i].gScore = INFINITY;
		nodes[i].currState.RemoveAll();
		nodes[i].goalState.RemoveAll();
	}
	Node& node = nodes.Tail();
	node.goalState.Insert(goal.m_key, goal.m_value);
	node.currState.Insert(goal.m_key, worldState[worldState.Find(goal.m_key)]);
	addToOpenSet(&(node));
	node.gScore = 0.0f;
	node.fScore = getHeuristicCost(node);
}

bool AStar::searchStep() {
	Node& current = *(openSet.ElementAtHead());
	openSet.RemoveAtHead();
	GoalState goal;
	bool foundGoal = false;
	FOR_EACH_MAP_FAST(current.goalState, i) {
		goal.m_key = current.goalState.Key(i);
		goal.m_value = current.goalState[i];
		if (goal.m_value
				!= current.currState[current.currState.Find(goal.m_key)]) {
			foundGoal = true;
			break;
		}
	}
	if (!foundGoal) { // no goals need to be satisfied
		if (current.parent != -1) {
			start.AddToTail(&current);
		}
		return openSet.Count() == 0;
	}
	current.isClosed = true;
	current.isOpen = false;
	const auto& efxIdx = efxToActions.Find(goal);
	if (!efxToActions.IsValidIndex(efxIdx)) {
		// no actions can satisfy this precondition.
		return openSet.Count() == 0;
	}
	CUtlVector<int> neighbors;
	getNeighbors(neighbors, efxToActions[efxIdx]);
	FOR_EACH_VEC(neighbors, i)
	{
		Node& neighbor = nodes[neighbors[i]];
		Action* action = actions[neighbor.id];
		if (neighbor.isClosed) {
			continue;
		}
		if (addToOpenSet(&neighbor)) {
			// found new node
			copy(neighbor.currState, current.currState);
			copy(neighbor.goalState, current.goalState);
			WorldState& currState = neighbor.currState;
			currState[currState.Find(goal.m_key)] = goal.m_value;
			const WorldState& precond = action->getPrecond();
			WorldState& goalState = neighbor.goalState;
			bool goalStateConflict = false;
			FOR_EACH_MAP_FAST(precond, j) {
				WorldProp prop = precond.Key(j);
				auto k = goalState.Find(prop);
				if (!goalState.IsValidIndex(k)) {
					goalState.Insert(prop, precond[j]);
					currState.Insert(prop, worldState[worldState.Find(prop)]);
				} else if (precond[j] != goalState[k]) {
					goalStateConflict = true;
					break;
				}
			}
			if (goalStateConflict) {
				// throw exception ?
				continue;
			}
		}
		float tentativeGScore = current.gScore + getEdgeCost(action);
		if (tentativeGScore >= neighbor.gScore) {
			continue;
		}
		neighbor.parent = current.id;
		neighbor.gScore = tentativeGScore;
		neighbor.fScore = neighbor.gScore + getHeuristicCost(neighbor);
	}
	return openSet.Count() == 0;
}

void AStar::getPath(CUtlQueue<int>& path) const {
	path.RemoveAll();
	if (start.IsEmpty()) {
		return;
	}
	float lowestCost = INFINITY;
	int lowestStart = -1;
	FOR_EACH_VEC(start, i) {
		float cost = 0.0f;
		for (int j = start[i]->id; j != nodes.Count() - 1; j = nodes[j].parent) {
			cost += actions[nodes[j].id]->getCost();
		}
		if (cost < lowestCost) {
			lowestStart = i;
			lowestCost = cost;
		}
	}
	for (int i = start[lowestStart]->id; i != nodes.Count() - 1; i = nodes[i].parent) {
		actions[nodes[i].id]->init();
		path.Insert(nodes[i].id);
	}
}

void AStar::getNeighbors(CUtlVector<int>& neighbors,
		const CCopyableUtlVector<int>& availableNeighbors) const {
	FOR_EACH_VEC(availableNeighbors, i)
	{
		Action& availableAction = *(actions[availableNeighbors[i]]);
		if (!availableAction.precondCheck()) {
			continue;
		}
		bool addAction = true;
		FOR_EACH_VEC(neighbors, j)
		{
			Action& neighbor = *(actions[neighbors[j]]);
			if (availableAction.getPrecond() == neighbor.getPrecond()) {
				addAction = availableAction.getCost() < neighbor.getCost();
				if (addAction) {
					neighbors.Remove(j);
				}
				break;
			}
		}
		if (addAction) {
			neighbors.AddToTail(availableNeighbors[i]);
		}
	}
}

void AStar::copy(WorldState& left, const WorldState& right) {
	if (&left == &right) {
		return;
	}
	left.RemoveAll();
	FOR_EACH_MAP_FAST(right, i) {
		left.Insert(right.Key(i), right[i]);
	}
}

void AStar::createNode() {
	nodes.AddToTail();
	SetDefLessFunc(nodes.Tail().goalState);
	SetDefLessFunc(nodes.Tail().currState);
	nodes.Tail().id = nodes.Count() - 1;
}

bool AStar::addToOpenSet(Node* node) {
	if (!node->isOpen) {
		node->isOpen = true;
		openSet.Insert(node);
		return true;
	}
	return false;
}

float AStar::getHeuristicCost(const Node& node) const {
	float cost = 0.0f;
	const auto& goals = node.goalState;
	FOR_EACH_MAP_FAST(goals, i) {
		WorldProp prop = goals.Key(i);
		const auto& currState = node.currState;
		const auto& j = currState.Find(prop);
		if (!currState.IsValidIndex(j)) {
			throw new SimpleException("Unable to find current state.");
		}
		if (goals[i] != currState[j]) {
			cost += 1.0f;
		}
	}
	return cost;
}

float AStar::getEdgeCost(Action * action) const {
	auto& precond = action->getPrecond();
	FOR_EACH_MAP_FAST(precond, j) {
		GoalState goal;
		goal.m_key = precond.Key(j);
		goal.m_value = precond[j];
		if (!efxToActions.IsValidIndex(efxToActions.Find(goal))) {
			// no actions can satisfy this precondition.
			return INFINITY;
		}
	}
	return action->getCost();
}
