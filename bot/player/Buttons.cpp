#include "Buttons.h"

#include <in_buttons.h>
#include <player/Button.h>

const static int BUTTONS = 26;

Buttons::Buttons() {
	SetDefLessFunc(buttons);
	for (int i = 0; i < BUTTONS; i++) {
		buttons.Insert(1 << i, Button());
	}
}

bool Buttons::jump() {
	if (buttons[buttons.Find(IN_JUMP)].press(10, 20)) {
		buttons[buttons.Find(IN_DUCK)].press(10, 20);
		return true;
	}
	return false;
}

bool Buttons::hold(int button) {
	return buttons[buttons.Find(button)].press();
}

bool Buttons::tap(int button) {
	return buttons[buttons.Find(button)].press(1, 10);
}

int Buttons::getPressed() {
	int pressed = 0;
	for (int i = 0; i < BUTTONS; i++) {
		int id = 1 << i;
		int idx = buttons.Find(id);
		if (buttons[idx].checkPressed()) {
			pressed |= id;
		}
	}
	return pressed;
}
