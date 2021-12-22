#include "PlannerTest.h"

#include <goap/WorldCond.h>
#include <goap/action/Action.h>
#include <goap/Planner.h>

std::queue<int> plan;
WorldState worldState;
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
		precond[WorldProp::OUT_OF_AMMO] = false;
		precond[WorldProp::ENEMY_SIGHTED] = false;
	}

} reload;

//1
class FindCoverAction: public TestAction {
public:
	FindCoverAction() :
			TestAction( { WorldProp::ENEMY_SIGHTED, false }) {
		precond[WorldProp::OUT_OF_AMMO] = false;
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
class KillAction: public TestAction {
public:
	KillAction() :
			TestAction( { WorldProp::ENEMY_SIGHTED, false }) {
		precond[WorldProp::WEAPON_IN_RANGE] = true;
		precond[WorldProp::WEAPON_LOADED] = true;
	}
} killEnemy;

void test(WorldProp prop, bool cond) {
	GoalState goal = { prop, cond };
	planner.startSearch(goal);
	while (!planner.searchStep())
		;
	planner.getPath(plan);
}

void PlannerTest::setUp() {
	worldState[WorldProp::USING_BEST_WEAP] = false;
	worldState[WorldProp::OUT_OF_AMMO] = false;
	worldState[WorldProp::WEAPON_LOADED] = false;
	worldState[WorldProp::WEAPON_IN_RANGE] = true;
	worldState[WorldProp::MULTIPLE_ENEMY_SIGHTED] = false;
	worldState[WorldProp::ENEMY_SIGHTED] = true;
}

void PlannerTest::tearDown() {
	plan = std::queue<int>();
	worldState.clear();
}

void PlannerTest::testMap() {
	MyUnorderedMap<WorldProp, int> testMap;
	testMap[WorldProp::USING_BEST_WEAP] = 0;
	testMap[WorldProp::ENEMY_SIGHTED] = 1;
	testMap[WorldProp::MULTIPLE_ENEMY_SIGHTED] = 2;
	TS_ASSERT_EQUALS(0, testMap[WorldProp::USING_BEST_WEAP]);
	TS_ASSERT_EQUALS(2, testMap[WorldProp::MULTIPLE_ENEMY_SIGHTED]);
	TS_ASSERT_EQUALS(1, testMap[WorldProp::ENEMY_SIGHTED]);
	auto testMap2 = testMap;
	for (auto i: testMap) {
		TS_ASSERT_EQUALS(i.second, testMap2.at(i.first))
	}
}

void PlannerTest::testSwitchToLoadedWeaponThenAttack() {
	test(WorldProp::ENEMY_SIGHTED, false);
	TS_ASSERT_EQUALS(2, plan.size());
	TS_ASSERT_EQUALS(2, plan.front());
	plan.pop();
	TS_ASSERT_EQUALS(3, plan.front());
	plan.pop();
}

void PlannerTest::testFindFindCoverReload() {
	switchToLoadedWeap.setProcPrecond(false);
	test(WorldProp::WEAPON_LOADED, true);
	TS_ASSERT_EQUALS(2, plan.size());
	TS_ASSERT_EQUALS(1, plan.front());
	plan.pop();
	TS_ASSERT_EQUALS(0, plan.front());
	plan.pop();
	switchToLoadedWeap.setProcPrecond(true);
}
