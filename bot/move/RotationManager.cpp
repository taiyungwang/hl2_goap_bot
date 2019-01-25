#include "RotationManager.h"

#include <cmath>

float RotationManager::clamp180(float angle) {
	if (std::abs(angle) > 360.0f) {
		angle -= 360.0f * (angle / 360);
	}
	if (angle > 180.0f) {
		angle -= 360.0f;
	} else if (angle < -180.0f) {
		angle += 360.0f;
	}
	return angle;
}

float RotationManager::getUpdatedPosition(float target, float current, float accel) {
	target = clamp180(target);
	float dist = clamp180(target - current);
	float speed = lastPos - current;
	lastPos = current;
	if (std::abs(dist) < accel + std::abs(speed)) {
		// if distance covered is less than acceleration + current speed, set to target, and reduce speed to 0
		speed = 0.0f;
		return target;
	}
	if (dist < 0.0f) {
		// we accelerating towards the negative side.
		accel = -accel;
	}
	// speed is opposite desired direction
	if (accel < 0.0f != speed < 0.0f
	// or we can still accelerate using Gaussian summation as estimation.
			|| std::abs(speed * (speed + accel)) / 2.0f < std::abs(dist)) {
		speed += accel;
	}
	return current + speed;
}
