#include "PlannerTest.h"

#include <goap/AStar.h>
#include <goap/action/Action.h>
#include <goap/WorldCond.h>
#include <utlvector.h>
#include <utlqueue.h>

CUtlQueue<int> plan;
CUtlMap<WorldProp, bool> worldState;
CUtlVector<Action*> actions;
AStar* planner;

class Blackboard {
};

class TestAction: public Action {
public:
	TestAction() :
			Action(bot) {
	}

	bool execute() {
		return true;
	}

private:
	Blackboard bot;
};

class LoadWeapon: public TestAction {
public:
	LoadWeapon() {
		effects = {WorldProp::WEAPON_LOADED, true};
	}
};

class ThrowGrenade: public TestAction {
public:
	ThrowGrenade() {
		effects = {WorldProp::ENEMY_SIGHTED, false};
		precond.Insert(WorldProp::MULTIPLE_ENEMY_SIGHTED, true);
	}
};
class Shoot: public TestAction {
public:
	Shoot() {
		effects = {WorldProp::ENEMY_SIGHTED, false};
		precond.Insert(WorldProp::WEAPON_LOADED, true);
	}
};

class FindCover: public TestAction {
public:
	FindCover() {
		effects = {WorldProp::ENEMY_SIGHTED, false};
	}

	float getCost() const {
		return 4.0f;
	}
};

class UseBestWeapon: public TestAction {
public:
	UseBestWeapon() {
		effects = {WorldProp::USING_BEST_WEAP, true};
	}
};

void test(WorldProp prop, bool cond) {
	GoalState goal = { prop, cond };
	planner->startSearch(goal);
	while (!planner->searchStep())
		;
	planner->getPath(plan);
}

PlannerTest::PlannerTest() {
	actions.AddToTail(new LoadWeapon());
	actions.AddToTail(new Shoot());
	actions.AddToTail(new UseBestWeapon());
	actions.AddToTail(new FindCover());
	actions.AddToTail(new ThrowGrenade());
	planner = new AStar(worldState);
	FOR_EACH_VEC(actions, i) {
		planner->addAction(actions[i]);
	}
}

PlannerTest::~PlannerTest() {
	delete planner;
	FOR_EACH_VEC(actions, i)
	{
		delete actions[i];
	}
	actions.RemoveAll();
}

void PlannerTest::setUp() {
	SetDefLessFunc(worldState);
	worldState.Insert(WorldProp::USING_BEST_WEAP, false);
	worldState.Insert(WorldProp::WEAPON_LOADED, false);
	worldState.Insert(WorldProp::MULTIPLE_ENEMY_SIGHTED, false);
	worldState.Insert(WorldProp::ENEMY_SIGHTED, true);
}

void PlannerTest::tearDown() {
	plan.RemoveAll();
	worldState.RemoveAll();
}

void PlannerTest::testNoPlan() {
	worldState[worldState.Find(WorldProp::USING_BEST_WEAP)] = true;
	test(WorldProp::USING_BEST_WEAP, true);
	TS_ASSERT_EQUALS(0, plan.Count());
}

void PlannerTest::testOneAction() {
	test(WorldProp::USING_BEST_WEAP, true);
	TS_ASSERT_EQUALS(1, plan.Count());
	TS_ASSERT_EQUALS(2, plan.RemoveAtHead());
}

void PlannerTest::testMultipleActions() {
	test(WorldProp::ENEMY_SIGHTED, false);
	TS_ASSERT_EQUALS(2, plan.Count());
	TS_ASSERT_EQUALS(0, plan.RemoveAtHead());
	TS_ASSERT_EQUALS(1, plan.RemoveAtHead());
}
