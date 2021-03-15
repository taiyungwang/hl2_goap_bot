#include "Buttons.h"

#include <in_buttons.h>

const static int BUTTONS = 26;

Buttons::Buttons() {
	for (int i = 0; i < BUTTONS; i++) {
		buttons.Insert(1 << i, Button());
	}
}

bool Buttons::jump() {
	if (buttons.GetPtr(IN_JUMP)->press(20, 10)) {
		buttons.GetPtr(IN_DUCK)->press(20, 10);
		return true;
	}
	return false;
}

bool Buttons::hold(int button) {
	return buttons.GetPtr(button)->press();
}

bool Buttons::tap(int button) {
	return buttons.GetPtr(button)->press(1, 10);
}

int Buttons::getPressed() {
	int pressed = 0;
	for (int i = 0; i < BUTTONS; i++) {
		int id = 1 << i;
		if (buttons.GetPtr(id)->checkPressed()) {
			pressed |= id;
		}
	}
	return pressed;
}

