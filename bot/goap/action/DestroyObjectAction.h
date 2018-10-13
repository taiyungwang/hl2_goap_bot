#pragma once

#include "Action.h"
#include <utllinkedlist.h>

class MoveStateContext;
class WeaponFunction;
struct edict_t;

class DestroyObjectAction: public Action {
public:
	DestroyObjectAction(Blackboard& blackboard);

	virtual ~DestroyObjectAction();

	void init() {
		crouch = false;
	}

	virtual bool precondCheck();

	bool execute();

	virtual bool postCondCheck();

protected:
	virtual bool isVisible(const Vector& end, edict_t* target) const {
		return true;
	}

	virtual void getAdjustedTargetPos(Vector& targetLoc,
			WeaponFunction* weapFunc) const;

	virtual edict_t* getTargetedEdict() const;

	virtual bool isBreakable(edict_t* object) const {
		return false;
	}

private:
	CUtlLinkedList<CUtlString> objectNames;

	int waitTime, maxWaitTime;

	MoveStateContext* moveCtx;

	bool crouch;
};
