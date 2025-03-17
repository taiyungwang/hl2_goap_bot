#pragma once

#include "GoToAction.h"

class HidingSpotSelector;

class SnipeAction: public GoToAction {
public:
	static void setSpotSelector(HidingSpotSelector* sel) {
		selector = sel;
	}

	SnipeAction(Bot *self);

	virtual bool precondCheck() override;

	bool init();

	bool execute();

	bool goalComplete();

	void abort();

	float getChanceToExec() const;

protected:
	static HidingSpotSelector* selector;

	bool crouch = true;

	int duration = 0;

	int selectorId = -1;

	QAngle facing;

	float calculateFacing();
};
