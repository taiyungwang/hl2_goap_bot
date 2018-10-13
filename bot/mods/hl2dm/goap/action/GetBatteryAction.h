#pragma once

#include <goap/action/GetItemAction.h>

/**
 * Defines the action for picking up a battery.
 */
class GetBatteryAction: public GetItemAction {
public:
	GetBatteryAction(Blackboard& blackboard) :
			GetItemAction("item_battery", blackboard) {
		effects = {WorldProp::ARMOR_FULL, true};
	}

	bool precondCheck();
};
