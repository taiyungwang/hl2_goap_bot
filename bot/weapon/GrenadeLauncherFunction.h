#pragma once

#include "WeaponFunction.h"

class GrenadeLauncherFunction: public WeaponFunction {
public:
	GrenadeLauncherFunction();

	Vector getAim(const Vector& target, const Vector& eye) const;

	void setInitialVelocity(float initialVelocity) {
		this->initialVelocity = initialVelocity;
	}

	virtual void attack(Buttons& buttons, float distance) const;

protected:
	float initialVelocity = 500.0f;
};
