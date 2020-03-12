#pragma once

#include <cxxtest/TestSuite.h>

class RotationManager;
class QAngle;

class RotationManagerTest: public CxxTest::TestSuite {
public:

	void setUp();

	void tearDown();

	void testNoDirChange();

	void testNegDir();

	void testDirChange();

	void testOverShootWhileSpeedingUp();

	void testOverShootWhileBraking();

	void testNormalize();

private:
	QAngle *desired, *current, *goal;

	float accel = 1.0f;

	void update(RotationManager& rotation);
};

