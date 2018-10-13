#pragma once

#include <utlmap.h>

class Button;

/**
 * Manages the state of CBotCmd.button.
 */
class Buttons {
public:
	Buttons();

	bool tap(int button);

	/**
	 * Performs a jump crouch
	 */
	bool jump();

	bool hold(int button);

	/**
	 * Returns an OR'd value of all buttons pressed.  The button
	 * states are updated. This should be called once per frame.
	 */
	int getPressed();

private:
	CUtlMap<int, Button> buttons;
};
