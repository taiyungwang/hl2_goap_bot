#pragma once

#include <goap/action/GoToItemAction.h>

/**
 * Defines the action for using a charger.
 */
class ChargeAction: public GoToItemAction {
public:
	ChargeAction(const char* name, Blackboard& blackboard) :
			GoToItemAction(name, "CBaseAnimating", "m_flCycle", blackboard) {
	}

	void init();

	float getCost() const {
		return 2.0f + GoToItemAction::getCost();
	}

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
	bool isAvailable(edict_t *ent) const;

	bool isDepleted() const;
};
