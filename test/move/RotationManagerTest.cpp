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
	RotationManager rotation;
	rotation.getUpdatedPosition(*desired, *current, accel);
	TS_ASSERT_EQUALS(1.0f, desired->x)
	update(rotation);
	update(rotation);
	TS_ASSERT_EQUALS(6.0f, desired->x)
	update(rotation);
	update(rotation);
	update(rotation);
	TS_ASSERT_EQUALS(goal->x, desired->x)
}

void RotationManagerTest::testDirChange() {
	RotationManager rotation;
	rotation.getUpdatedPosition(*desired, *current, accel);
	goal->x = -10.0f;
	update(rotation);
	TS_ASSERT_EQUALS(1.0f, desired->x)
	update(rotation);
	TS_ASSERT_EQUALS(0.0f, desired->x)
}

void RotationManagerTest::testNegDir() {
	RotationManager rotation;
	desired->x = goal->x = -10.0f;
	rotation.getUpdatedPosition(*desired, *current, accel);
	TS_ASSERT_EQUALS(-1.0f, desired->x)
	update(rotation);
	update(rotation);
	update(rotation);
	desired->x = goal->x = -1.0f;
	update(rotation);
	TS_ASSERT_EQUALS(-2.0f, desired->x)
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
	*current = *desired;
	*desired = *goal;
	rotation.getUpdatedPosition(*desired, *current, accel);
}
