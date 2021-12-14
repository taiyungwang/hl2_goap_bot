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
	if (force || state != State::ACTION || plan.IsEmpty()
			|| actions[plan.Head()]->isInterruptable()) {
		if (state == State::ACTION && !plan.IsEmpty()) {
			actions[plan.Head()]->abort();
		}
		reset();
	}
}

void GoalManager::execute() {
	switch (state) {
	case State::PLANNING: {
		bool finished = false;
		extern IVEngineServer* engine;
		// timebox to 1/60 second
		float timeLimit = 0.0167f + engine->Time();
		while (!finished && engine->Time() <= timeLimit) {
			finished = planBuilder->searchStep();
		}
		if (finished) {
			planBuilder->getPath(plan);
			if (plan.IsEmpty()) {
				getNextGoal();
			} else {
				state = State::ACTION;
				actions[plan.Head()]->init();
			}
		}
		break;
	}
	case State::ACTION: {
		if (plan.IsEmpty()) {
			reset();
			break;
		}
		Action* action = actions[plan.Head()];
		if (action->execute()) {
			plan.RemoveAtHead();
			if (action->goalComplete() && !plan.IsEmpty()) {
				actions[plan.Head()]->init();
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
		getNextGoal();
	}
}

void GoalManager::reset() {
	state = State::REPLAN;
	currentGoal = 0;
}

void GoalManager::getNextGoal() {
	for (; currentGoal < goals.Count(); currentGoal++) {
		auto& goal = goals[currentGoal];
		auto& effect = actions[goal.action]->getEffects();
		float chanceToExec = actions[goal.action]->getChanceToExec();
		if (effect.m_value != worldState[worldState.Find(effect.m_key)]
			&& (chanceToExec >= 1.0f || chanceToExec > RandomFloat(0, 1.0f))) {
			break;
		}
	}
	if (currentGoal >= goals.Count()) {
		reset();
		return;
	}
	planBuilder->startSearch(
			actions[goals[currentGoal++].action]->getEffects());
	state = State::PLANNING;
}

void GoalManager::addAction(Action* action) {
	actions.AddToTail(action);
	planBuilder->addAction(action);
}
