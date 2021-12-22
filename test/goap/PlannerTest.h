#pragma once

#include <cxxtest/TestSuite.h>


class PlannerTest: public CxxTest::TestSuite {
public:
	void setUp();

	void tearDown();

	void testMap();

	void testSwitchToLoadedWeaponThenAttack();

	void testFindFindCoverReload();
};

