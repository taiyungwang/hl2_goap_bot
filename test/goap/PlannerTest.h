#pragma once

#include <cxxtest/TestSuite.h>


class PlannerTest: public CxxTest::TestSuite {
public:

	PlannerTest();

	~PlannerTest();

	void setUp();

	void tearDown();

	void testNoPlan();

	void testNoGoalFound();

	void testOneAction();

	void testMultipleActions();

	void testDeadEndReached();
};

