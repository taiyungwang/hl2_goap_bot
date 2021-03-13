#pragma once

#include "CapturePointAction.h"

class DODDefendPointAction: public CapturePointAction {
public:
	DODDefendPointAction(Blackboard& blackboard) :
		CapturePointAction(blackboard) {
		effects = {WorldProp::POINTS_DEFENDED, true};
	}

	virtual bool precondCheck();

	virtual bool execute();

protected:
	int duration = 0;

	virtual bool isAvailable(int idx);

	virtual void selectFromActive(CUtlLinkedList<edict_t*>& active);

private:
	unsigned enemyControlled = 0;
};
