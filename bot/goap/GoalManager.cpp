#include "GoalManager.h"

#include "Planner.h"
#include "action/Action.h"
#include <player/Player.h>
#include <vstdlib/random.h>
#include <algorithm>

GoalManager::GoalManager(const WorldState& worldState, Blackboard& blackboard) :
		blackboard(blackboard), worldState(worldState) {
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
	if (!force && !plan.empty() && !actions[plan.front()]->isInterruptable()) {
		return;
	}
	if (!plan.empty()) {
		actions[plan.front()]->abort();
	}
	reset();
}

void GoalManager::execute() {
	while (plan.empty() && getNextGoal()) {
		if (planBuilder->findPlan(actions[goals[currentGoal++].action]->getEffects())) {
			planBuilder->buildPlan(plan);
			if (!plan.empty()) {
				if (!actions[plan.front()]->init()) {
					reset();
					return;
				}
			}
		}
	}
	if (!plan.empty() && actions[plan.front()]->execute()) {
		if (!actions[plan.front()]->goalComplete()) {
			reset();
		} else {
			plan.pop();
			if (plan.empty() || !actions[plan.front()]->init()) {
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

void GoalManager::addAction(float priority, Action* action) {
	actions.push_back(action);
	planBuilder->addAction(action);
	if (priority <= 0.0f) {
		return;
	}
	goals.emplace_back();
	goals.back().action = actions.size() - 1;
	goals.back().priority = priority;
	std::sort(goals.begin(), goals.end(),
			[](const Goal &g1, const Goal &g2) -> bool {
				return g1.priority > g2.priority;
			});
}
