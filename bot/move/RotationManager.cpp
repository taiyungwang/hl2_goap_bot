#include "RotationManager.h"

#include <convar.h>
#include <vector.h>
#include <cmath>

ConVar mybot_rot_speed("mybot_rot_speed", "0.3", 0,
		"determines rotational acceleration rate in degrees");

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
	angle.x = Clamp(angle.x, -89.0f, 89.0f);
}

void RotationManager::getUpdatedPosition(QAngle &desiredPos, QAngle currentPos) {
	desiredPos -= currentPos;
	normalize(desiredPos);
	normalize(currentPos);
	desiredPos.x = getUpdatedPos(momentum->x, desiredPos.x, currentPos.x);
	desiredPos.y = getUpdatedPos(momentum->y, desiredPos.y, currentPos.y);
	// need to normalize again in case the rotation for y axis is something like -170 to 180 along
	normalize(desiredPos);
}

float RotationManager::getUpdatedPos(float &speed, float desiredSpeed,
		float currentPos) {
	float accelMagnitude = mybot_rot_speed.GetFloat();
	float accel = desiredSpeed > 0.0f ? accelMagnitude : -accelMagnitude;
	// almost stopped
	if (std::abs(speed) <= accelMagnitude
			// magnitude of desired speed is less than that of acceleration.
			&& std::abs(desiredSpeed) <= accelMagnitude
			&& (isSameDir(desiredSpeed, speed)
					|| std::abs(speed + accel) >= std::abs(desiredSpeed))) {
		speed = 0.0f;
		return desiredSpeed + currentPos;
	}
	// want to accelerate
	if (isSameDir(speed, desiredSpeed)) {
		// estimated distance to decelerate is less than amount of distance left
		if (speed * speed / accelMagnitude * 0.5f >= std::abs(desiredSpeed)) {
			accel = -accel;
		// accelerating in the same direction will cause us to overshoot in subsequent frames.
		} else if (isSameDir(speed, accel)
				&& powf(speed + accel, 2.0f) / accelMagnitude * 0.5 > std::abs(desiredSpeed)) {
			accel = 0.0f;
		}
	}
	speed += accel;
	return speed + currentPos;
}
