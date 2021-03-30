#pragma once

#include "GoToItemAction.h"

/**
 * Defines the action for using a charger.
 */
class ChargeAction: public GoToItemAction {
public:
	ChargeAction(const char* name, Blackboard& blackboard) :
			GoToItemAction(name, "CBaseAnimating", "m_flCycle", blackboard) {
	}

	float getCost() {
		return 2.0f + GoToAction::getCost();
	}

	bool precondCheck();

	bool execute();

	bool isInterruptable() const {
		return true;
	}

protected:
	/**
	 * @return True if charging action is no longer needed.
	 */
	virtual bool isFinished() const = 0;

private:
	int chargeTime = 0;

	bool isAvailable(edict_t *ent);

	bool findTargetLoc();

	bool isDepleted();
};
