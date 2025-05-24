#pragma once

#include <goap/action/UseSpecificWeaponAction.h>
#include <vector.h>

class Player;

class ThrowGrenadeAction: public UseSpecificWeaponAction {
public:
	ThrowGrenadeAction(Bot *self) :
			UseSpecificWeaponAction(self) {
		effects = {WorldProp::MULTIPLE_ENEMY_SIGHTED, false};
	}

	bool precondCheck() override;

protected:
	Vector target;

	virtual const Player *chooseTarget() const;

	virtual bool canUse(const char *weapName) const = 0;

	bool use() override;

private:
	bool canUse(int weapIndx) const override;
};
