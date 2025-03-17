#pragma once

#include "GoToAction.h"

class EntityVar;
struct edict_t;

/**
 * Defines an action to go to a class of entity on the map.
 */
class GoToEntityAction: public GoToAction {
public:
	virtual bool precondCheck() override;

	virtual bool init();

	virtual bool execute() override;

protected:
	edict_t* item = nullptr;

	GoToEntityAction(Bot *self): GoToAction(self) {
	}

	virtual void selectItem() = 0;

	void setTargetLocAndRadius(edict_t* target);

	void useItem(bool isActive);
};
