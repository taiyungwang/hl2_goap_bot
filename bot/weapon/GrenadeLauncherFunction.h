#pragma once

#include "WeaponFunction.h"

class GrenadeLauncherFunction: public WeaponFunction {
public:
	GrenadeLauncherFunction();

	Vector getAim(const Vector& target, const Vector& eye) const;

	void setZMultiplier(float zMultiplier) {
		this->zMultiplier = zMultiplier;
	}

	virtual void attack(Buttons& buttons, float distance) const;

protected:
	float zMultiplier = 100.0f;
};
