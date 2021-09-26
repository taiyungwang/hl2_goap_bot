#include "RotationManagerTest.h"

#include <bot/move/RotationManager.h>
#include <vector.h>

void RotationManagerTest::setUp() {
	desired = new QAngle(10.0f, 0.0f, 0.0f);
	current = new QAngle(0.0f, 0.0f, 0.0f);
	goal = new QAngle(*desired);
}

void RotationManagerTest::tearDown() {
	delete desired;
	delete current;
	delete goal;
}

void RotationManagerTest::testNoDirChange() {
	float expectedPosition[] = { 1.0f, 3.0f, 6.0f, 8.0f, 9.0f, 10.0f, 10.0f };
	test(expectedPosition, sizeof(expectedPosition) / sizeof(float));
}

void RotationManagerTest::testDirChange() {
	RotationManager rotation;
	update(rotation);
	desired->x = goal->x = -10.0f;
	update(rotation);
	TS_ASSERT_EQUALS(1.0f, current->x)
	update(rotation);
	TS_ASSERT_EQUALS(0.0f, current->x)
}

void RotationManagerTest::testNegDir() {
	desired->x = goal->x = -10.0f;
	float expected[] = { -1.0f, -3.0f, -6.0f, -8.0f, -9.0f, -10.0f, -10.0f };
	test(expected, sizeof(expected) / sizeof(float));
}

void RotationManagerTest::testOverShootWhileSpeedingUp() {
	desired->x = goal->x = 0.5f;
	RotationManager().getUpdatedPosition(*desired, *current, accel);
	TS_ASSERT_EQUALS(goal->x, desired->x);
}

void RotationManagerTest::testOverShootWhileBraking() {
	desired->x = goal->x = 1.5f;
	RotationManager rotation;
	rotation.getUpdatedPosition(*desired, *current, accel);
	update(rotation);
	TS_ASSERT_EQUALS(goal->x, desired->x)
}

void RotationManagerTest::testNormalize() {
	desired->y = goal->y = 541.0f;
	desired->x = goal->x = 91.0f;
	RotationManager().getUpdatedPosition(*desired, *current, accel);
	TS_ASSERT_EQUALS(-1.0f, desired->y)
	TS_ASSERT_EQUALS(1.0f, desired->x)
}

void RotationManagerTest::update(RotationManager& rotation) {
	rotation.getUpdatedPosition(*desired, *current, accel);
	*current = *desired;
	*desired = *goal;
}

void RotationManagerTest::test(float position[], int positionLen) {
	RotationManager rotation;
	for (int i = 0; i < positionLen; i++) {
		update(rotation);
		TS_ASSERT_EQUALS(position[i], current->x);
	}
}
