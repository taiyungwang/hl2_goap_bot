#include "Planner.h"

#include "AStar.h"
#include "action/Action.h"
#include <player/Player.h>
#include <eiface.h>

Planner::Planner(const WorldState& worldState, Blackboard& blackboard) :
		worldState(worldState), blackboard(blackboard) {
	planBuilder = new AStar(worldState);
}

Planner::~Planner() {
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

void Planner::resetPlanning(bool force) {
	if (force || state != State::ACTION || plan.IsEmpty()
			|| actions[plan.Head()]->isInterruptable()) {
		reset();
	}
}


void Planner::execute() {
	switch (state) {
	case State::PLANNING: {
		bool finished = false;
		extern IVEngineServer* engine;
		// timebox to 1/60 second
		float timeLimit = 0.0167f + engine->Time();
		while (!finished) {
			if (engine->Time() > timeLimit) {
				break;
			}
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
			if (!action->postCondCheck()) {
				getNextGoal();
			} else if (!plan.IsEmpty()) {
				actions[plan.Head()]->init();
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

void Planner::reset() {
	state = State::REPLAN;
	currentGoal = 0;
}

void Planner::getNextGoal() {
	for (; currentGoal < goals.Count(); currentGoal++) {
		auto& effects = actions[goals[currentGoal].action]->getEffects();
		if (effects.m_value != worldState[worldState.Find(effects.m_key)]) {
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

void Planner::addAction(Action* action) {
	actions.AddToTail(action);
	planBuilder->addAction(action);
}
