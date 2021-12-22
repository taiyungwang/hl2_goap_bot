#pragma once

#include <goap/action/UseSpecificWeaponAction.h>
#include <vector.h>

class Player;

class ThrowGrenadeAction: public UseSpecificWeaponAction {
public:
	ThrowGrenadeAction(Blackboard &blackboard) :
			UseSpecificWeaponAction(blackboard) {
		effects = {WorldProp::MULTIPLE_ENEMY_SIGHTED, false};
	}

	virtual bool precondCheck() override;

	virtual bool execute() override;

protected:
	Vector target;

	virtual const Player *chooseTarget() const;
};
