#pragma once

#include <cxxtest/TestSuite.h>


class PlannerTest: public CxxTest::TestSuite {
public:
	void setUp();

	void tearDown();

	void testSwitchToLoadedWeaponThenAttack();

	void testFindFindCoverReload();
};

