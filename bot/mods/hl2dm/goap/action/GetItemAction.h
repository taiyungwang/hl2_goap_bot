#pragma once

#include "GoToItemAction.h"

/**
 * Defines the action for getting an item that can be picked up.
 */
class GetItemAction: public GoToItemAction {
protected:
	GetItemAction(const char* name, Blackboard& blackboard) :
			GoToItemAction(name, "CBaseEntity", "m_fEffects", blackboard) {
	}

private:
	bool isAvailable(edict_t *) const;

	bool isDepleted() const {
		return !isAvailable(item);
	}
};
