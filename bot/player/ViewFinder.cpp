#include "ViewFinder.h"

#include "Blackboard.h"
#include <convar.h>

ConVar mybot_mouse_speed("mybot_mouse_speed", "0.3f", 0, "0.0 to 1.0 exclusive");

float ViewFinder::getActualSpeed(float ideal, float current) {
	ideal = Blackboard::clamp180(ideal);
	float anglespeed = 0.21f;
	float alphaspeed = mybot_mouse_speed.GetFloat();
	float delta = alphaspeed * anglespeed * Blackboard::clamp180(ideal - current)
			+ (momentum * alphaspeed);
	//check for QNAN
	if (delta != delta)
		delta = 1.0f;
	momentum = momentum * alphaspeed + delta * (1.0f - alphaspeed);
	//check for QNAN
	if (momentum != momentum)
		momentum = 1.0f;
	return Blackboard::clamp180(current + delta);
}

void ViewFinder::updateAngle(QAngle& target, const QAngle& current) {
	target.y = getActualSpeed(target.y, current.y);
	target.x = Clamp(getActualSpeed(target.x, current.x), -89.0f, 89.0f);
}
