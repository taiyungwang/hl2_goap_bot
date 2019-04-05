#pragma once

#include "ChargeAction.h"

/**
 * Defines the action for using a suit charger
 */
class ChargeArmorAction: public ChargeAction {
public:
	ChargeArmorAction(Blackboard& blackboard) :
			ChargeAction("item_suitcharger", blackboard) {
		effects = {WorldProp::ARMOR_FULL, true};
	}

private:
	bool isFinished() const;
};
