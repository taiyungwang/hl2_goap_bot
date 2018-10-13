#pragma once

#include <weapon/GrenadeLauncherFunction.h>

class DODFragGrenadeFunction: public GrenadeLauncherFunction {
public:
	DODFragGrenadeFunction();

	void attack(Buttons& buttons, float distance) const;

};
