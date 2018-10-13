#include <player/Button.h>

Button::Button() {
	pressed = false;
	duration = coolDown = 0;
}

bool Button::press(unsigned int duration, unsigned int coolDown) {
	if (pressed || this->coolDown > 0) {
		return false;
	}
	pressed = true;
	this->duration = duration;
	this->coolDown = coolDown;
	return true;
}

bool Button::checkPressed() {
	bool ret = pressed;
	if (pressed && duration == 0) {
		pressed = false;
	}
	if (duration > 0) {
		duration--;
	}
	if (coolDown > 0) {
		coolDown--;
	}
	return ret;
}
