#pragma once

class QAngle;

class ViewFinder {
public:

	void reset() {
		momentum = 0.0f;
	}
	/**
	 * Updates the current angle towards the targetAngle.
	 * @param angle Current angle
	 * @param targetAngle desired angle
	 */
	void updateAngle(QAngle& target, const QAngle& current);

private:
	static void clamp180(QAngle& angle);

	float momentum = 0.0f;

	float getActualSpeed(float idealSpeed, float currentSpeed);
};
