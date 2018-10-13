#include "ViewFinder.h"

#include "Blackboard.h"
#include <convar.h>


ConVar mybot_mouse_speed("mybot_mouse_speed", "5.0f");

void ViewFinder::updateAngle(QAngle& angle, QAngle targetAngle) {
	clamp180(targetAngle);
	QAngle delta = targetAngle - angle;
	float leng = delta.Length();
	if (leng <= 0.0f) {
		return;
	}
	delta *= (MIN(leng, mybot_mouse_speed.GetFloat())) / leng;
	clamp180(delta);
	angle += delta;
	clamp180(angle);
	Clamp(angle.x, -89.0f, 89.0f);
}

void ViewFinder::clamp180(QAngle& angle) {
	angle.x = Blackboard::clamp180(angle.x);
	angle.y = Blackboard::clamp180(angle.y);
}
