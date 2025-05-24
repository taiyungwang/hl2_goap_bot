#pragma once

class QAngle;

/**
 * Manages the rotation of the bot.  Uses acceleration and momentum to smooth
 * out the rotation.  The acceleration is constant in both directions, therefore We can then
 * determine whether we need to brake by treating the amount of distance covered as
 * the area of a right triangle and the two sides along the right angle as the current
 * speed and speed / acceleration, respectively.
 */
class RotationManager {
public:
	static float clamp180(float angle);

	RotationManager();

	~RotationManager();

	/**
	 * @param desiredPos The position the bots want to turned to.
	 */
	void getUpdatedPosition(QAngle& desiredPos, QAngle currentPos);

private:
	static void normalize(QAngle& angle);

	static bool isSameDir(float f1, float f2) {
		return (f1 >= 0) == (f2 >= 0);
	}

	/**
	 * Tracks the current rotational momentum of the bot.
	 */
	QAngle* momentum;

	/**
	 * Updates momentum and calculate the new position.
	 */
	static float getUpdatedPos(float& speed, float desired, float currentPos);
};
