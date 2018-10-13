#pragma once

class QAngle;

class ViewFinder {
public:

	/**
	 * Updates the current angle towards the targetAngle.
	 * @param angle Current angle
	 * @param targetAngle desired angle
	 */
	static void updateAngle(QAngle& angle, QAngle targetAngle);

private:
	static void clamp180(QAngle& angle);
};
