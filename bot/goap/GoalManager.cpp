#include "GoalManager.h"

#include "Planner.h"
#include "action/Action.h"
#include <player/Player.h>
#include <eiface.h>
#include <vstdlib/random.h>

GoalManager::GoalManager(const WorldState& worldState, Blackboard& blackboard) :
		worldState(worldState), blackboard(blackboard) {
	planBuilder = new Planner(worldState);
}

GoalManager::~GoalManager() {
	if (planBuilder != nullptr) {
		delete planBuilder;
	}
	FOR_EACH_VEC(actions, i)
	{
		delete actions[i];
	}
	actions.RemoveAll();
	goals.RemoveAll();
}

void GoalManager::resetPlanning(bool force) {
	if (force || state != State::ACTION || plan.empty()
			|| actions[plan.front()]->isInterruptable()) {
		if (state == State::ACTION && !plan.empty()) {
			actions[plan.front()]->abort();
		}
		reset();
	}
}

void GoalManager::execute() {
	switch (state) {
	case State::PLANNING: {
		extern IVEngineServer* engine;
		plan = std::queue<int>();
		while (plan.empty() && getNextGoal()) {
			// timebox to 1/60 second
			float timeLimit = 0.0167f + engine->Time();
			bool finished = false;
			while (!finished && engine->Time() <= timeLimit) {
				finished = planBuilder->searchStep();
			}
			if (finished) {
				planBuilder->getPath(plan);
				if (!plan.empty()) {
					state = State::ACTION;
					actions[plan.front()]->init();
				}
			}
		}
		break;
	}
	case State::ACTION: {
		if (plan.empty()) {
			reset();
			break;
		}
		Action* action = actions[plan.front()];
		if (action->execute()) {
			plan.pop();
			if (action->goalComplete() && !plan.empty()) {
				actions[plan.front()]->init();
			} else {
				reset();
			}
		}
		break;
	}
	default: // REPLAN
		goals.Sort([] (const Goal* g1, const Goal* g2) {
			if (g2->priority == g1->priority) {
				return 0;
			}
			return g2->priority < g1->priority ? -1: 1;
		});
		state = State::PLANNING;
	}
}

void GoalManager::reset() {
	state = State::REPLAN;
	currentGoal = 0;
}

bool GoalManager::getNextGoal() {
	for (; currentGoal < goals.Count(); currentGoal++) {
		auto& goal = goals[currentGoal];
		auto& effect = actions[goal.action]->getEffects();
		float chanceToExec = actions[goal.action]->getChanceToExec();
		if (std::get<1>(effect) != worldState.at(std::get<0>(effect))
			&& (chanceToExec >= 1.0f || chanceToExec > RandomFloat(0, 1.0f))) {
			break;
		}
	}
	if (currentGoal >= goals.Count()) {
		reset();
		return false;
	}
	planBuilder->startSearch(actions[goals[currentGoal++].action]->getEffects());
	return true;
}

void GoalManager::addAction(Action* action) {
	actions.AddToTail(action);
	planBuilder->addAction(action);
}
