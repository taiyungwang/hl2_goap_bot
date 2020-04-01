#pragma once

class Button {
public:
	/**
	 * Checks to see if a button is pressed and updates the state.
	 * Assumed this is called once per frame.
	 */
	bool checkPressed();

	bool press(unsigned int duration = 0, unsigned int coolDown = 0);

private:

	bool pressed = false;

	unsigned int coolDown = 0,
			duration = 0;
};
