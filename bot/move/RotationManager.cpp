#include "RotationManager.h"

#include <vector.h>
#include <cmath>

float RotationManager::clamp180(float angle) {
	if (std::abs(angle) > 360.0f) {
		angle -= 360.0f * static_cast<int>(angle / 360.0f);
	}
	if (angle > 180.0f) {
		angle -= 360.0f;
	} else if (angle < -180.0f) {
		angle += 360.0f;
	}
	return angle;
}

RotationManager::RotationManager() :
		momentum(new QAngle(0, 0, 0)) {
}

RotationManager::~RotationManager() {
	delete momentum;
}

void RotationManager::normalize(QAngle &angle) {
	angle.x = clamp180(angle.x);
	angle.y = clamp180(angle.y);
	if (angle.x > 90.0f) {
		angle.x = 180.0f - angle.x;
	} else if (angle.x < -90.0f) {
		angle.x = -180.0f - angle.x;
	}
	Clamp(angle.x, -89.0f, 89.0f);
}

void RotationManager::getUpdatedPosition(QAngle &desiredPos, QAngle currentPos,
		float accelMagnitude) {
	this->accelMagnitude = accelMagnitude;
	desiredPos -= currentPos;
	normalize(desiredPos);
	normalize(currentPos);
	desiredPos.x = getUpdatedPos(momentum->x, desiredPos.x, currentPos.x);
	desiredPos.y = getUpdatedPos(momentum->y, desiredPos.y, currentPos.y);
}

float RotationManager::getUpdatedPos(float &speed, float desiredSpeed,
		float currentPos) {
	if (desiredSpeed == currentPos && speed == 0.0f) {
		return currentPos;
	}
	float accel = desiredSpeed >= speed ? accelMagnitude : -accelMagnitude;
	// do we need to brake
	if (std::abs(speed) > 0 && isSameSign(speed, desiredSpeed)
			&& speed * speed / accelMagnitude * 0.5f
					>= std::abs(desiredSpeed)) {
		// if we are not already braking
		if (isSameSign(accel, speed)) {
			accel = -accel;
		}
		// if we are close to stopping and current speed will cause us to overshoot target
		if (std::abs(speed) <= accelMagnitude
				&& accelMagnitude > std::abs(desiredSpeed)) {
			speed = 0;
			return currentPos + desiredSpeed;
		}
	}
	// if we are going to overshoot
	if (isSameSign(accel, speed)
			&& std::abs(accel + speed) > std::abs(desiredSpeed)) {
		currentPos += speed > 0 ? speed : desiredSpeed;
		speed = 0;
		return currentPos;
	}
	speed += accel;
	return currentPos + speed;
}
