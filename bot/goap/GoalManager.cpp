#include "GoalManager.h"

#include "Planner.h"
#include "action/Action.h"
#include <player/Player.h>
#include <vstdlib/random.h>

GoalManager::GoalManager(const WorldState& worldState, Blackboard& blackboard) :
		worldState(worldState), blackboard(blackboard) {
	planBuilder = new Planner(worldState);
}

GoalManager::~GoalManager() {
	if (planBuilder != nullptr) {
		delete planBuilder;
	}
	for(auto action: actions)
	{
		delete action;
	}
	actions.clear();
	goals.clear();
}

void GoalManager::resetPlanning(bool force) {
	if (!plan.empty() && (force || actions[plan.front()]->isInterruptable())) {
		actions[plan.front()]->abort();
		reset();
	}
}

void GoalManager::execute() {
	while (plan.empty() && getNextGoal()) {
		if (planBuilder->findPlan(actions[goals[currentGoal++].action]->getEffects())) {
			planBuilder->buildPlan(plan);
			if (!plan.empty()) {
				actions[plan.front()]->init();
			}
		}
	}
	if (!plan.empty() && actions[plan.front()]->execute()) {
		if (!actions[plan.front()]->goalComplete()) {
			reset();
		} else {
			plan.pop();
			if (!plan.empty()) {
				actions[plan.front()]->init();
			} else {
				reset();
			}
		}
	}
}

bool GoalManager::getNextGoal() {
	for (; currentGoal < goals.size(); currentGoal++) {
		auto& goal = goals[currentGoal];
		auto& effect = actions[goal.action]->getEffects();
		float chanceToExec = actions[goal.action]->getChanceToExec();
		if (std::get<1>(effect) != worldState.at(std::get<0>(effect))
			&& (chanceToExec >= 1.0f || chanceToExec > RandomFloat(0, 1.0f))) {
			return true;
		}
	}
	reset();
	return false;
}

void GoalManager::reset() {
	plan = std::queue<int>();
	currentGoal = 0;
}

void GoalManager::addAction(Action* action) {
	actions.push_back(action);
	planBuilder->addAction(action);
}
