#pragma once

#include "CapturePointAction.h"

class DODDefendPointAction: public CapturePointAction {
public:
	DODDefendPointAction(Blackboard& blackboard) :
		CapturePointAction(blackboard) {
		effects = {WorldProp::POINTS_DEFENDED, true};
		targetRadius = 50.0f;
	}

	virtual bool precondCheck();

	virtual bool execute() {
		return GoToAction::execute() && GoToAction::postCondCheck() && duration++ < 1000;
	}

protected:
	int duration = 0;

	virtual bool isAvailable(int idx) const;

	bool isBombInState(int idx, int state) const;

};
