#pragma once

/**
 * Manages a bot's rotation along an Euler axis.
 */
class RotationManager {
public:
	static float clamp180(float angle);

	float getUpdatedPosition(float target, float current, float accel);

private:
	float lastPos = 0.0f;
};

