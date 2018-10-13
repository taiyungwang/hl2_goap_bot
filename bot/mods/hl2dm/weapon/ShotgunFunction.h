#pragma once

#include <weapon/WeaponFunction.h>

class ShotgunFunction: public WeaponFunction {
public:
	ShotgunFunction(): WeaponFunction(0.96f) {
		range[1] = 768.0f;
	}

	void attack(Buttons& buttons, float distance) const;

};
