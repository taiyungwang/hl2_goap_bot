#pragma once

class Button {
public:
	Button();

	/**
	 * Checks to see if a button is pressed and updates the state.
	 * Assumed this is called once per frame.
	 */
	bool checkPressed();

	bool press(unsigned int duration = 0, unsigned int coolDown = 0);

private:

	bool pressed;

	unsigned int coolDown;

	unsigned int duration;
};
