#pragma once

#include "Button.h"
#include <strtools.h>
#include <utlhashtable.h>

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
	 * states are updated per frame.
	 */
	int getPressed();

private:
	CUtlHashtable<int, Button> buttons;
};
