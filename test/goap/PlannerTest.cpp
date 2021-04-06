#include "PlannerTest.h"

#include <goap/WorldCond.h>
#include <goap/action/Action.h>
#include <goap/Planner.h>
#include <utlqueue.h>

CUtlQueue<int> plan;
CUtlMap<WorldProp, bool> worldState;
Planner planner(worldState);

class Blackboard {
} bb;

class TestAction: public Action {
public:
	TestAction(const GoalState &effect) :
			Action(bb) {
		effects = effect;
		planner.addAction(this);
	}

	bool execute() {
		return true;
	}

	bool precondCheck() {
		return procPrecond;
	}

	void setProcPrecond(bool procPrecond) {
		this->procPrecond = procPrecond;
	}

private:
	bool procPrecond = true;
};

//0
class ReloadWeaponAction: public TestAction {
public:
	ReloadWeaponAction() :
			TestAction( { WorldProp::WEAPON_LOADED, true }) {
		precond.Insert(WorldProp::OUT_OF_AMMO, false);
		precond.Insert(WorldProp::ENEMY_SIGHTED, false);
	}

} reload;

//1
class FindCoverAction: public TestAction {
public:
	FindCoverAction() :
			TestAction( { WorldProp::ENEMY_SIGHTED, false }) {
		precond.Insert(WorldProp::OUT_OF_AMMO, false);
	}

	float getCost() {
		return 5.0f;
	}
} findCover;

//2
class SwitchToBestLoadedWeaponAction: public TestAction {
public:
	SwitchToBestLoadedWeaponAction() :
			TestAction( { WorldProp::WEAPON_LOADED, true }) {
	}
} switchToLoadedWeap;

//3
class DestroyObjectAction: public TestAction {
public:
	DestroyObjectAction() :
			TestAction( { WorldProp::ENEMY_SIGHTED, false }) {
		precond.Insert(WorldProp::WEAPON_IN_RANGE, true);
		precond.Insert(WorldProp::WEAPON_LOADED, true);
	}
} attack;

void test(WorldProp prop, bool cond) {
	GoalState goal = { prop, cond };
	planner.startSearch(goal);
	while (!planner.searchStep())
		;
	planner.getPath(plan);
}

void PlannerTest::setUp() {
	SetDefLessFunc(worldState);
	worldState.Insert(WorldProp::USING_BEST_WEAP, false);
	worldState.Insert(WorldProp::OUT_OF_AMMO, false);
	worldState.Insert(WorldProp::WEAPON_LOADED, false);
	worldState.Insert(WorldProp::WEAPON_IN_RANGE, true);
	worldState.Insert(WorldProp::MULTIPLE_ENEMY_SIGHTED, false);
	worldState.Insert(WorldProp::ENEMY_SIGHTED, true);
}

void PlannerTest::tearDown() {
	plan.RemoveAll();
	worldState.RemoveAll();
}

void PlannerTest::testSwitchToLoadedWeaponThenAttack() {
	test(WorldProp::ENEMY_SIGHTED, false);
	TS_ASSERT_EQUALS(2, plan.Count());
	TS_ASSERT_EQUALS(2, plan.RemoveAtHead());
	TS_ASSERT_EQUALS(3, plan.RemoveAtHead());
}

void PlannerTest::testFindFindCoverReload() {
	switchToLoadedWeap.setProcPrecond(false);
	test(WorldProp::WEAPON_LOADED, true);
	TS_ASSERT_EQUALS(2, plan.Count());
	TS_ASSERT_EQUALS(1, plan.RemoveAtHead());
	TS_ASSERT_EQUALS(0, plan.RemoveAtHead());
	switchToLoadedWeap.setProcPrecond(true);
}
