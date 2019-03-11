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

	bool adjustAim = true;

	unsigned int dur = 0;

	virtual edict_t* getTargetedEdict() const;

	virtual bool isBreakable(edict_t* object) const {
		return false;
	}

private:
	CUtlLinkedList<CUtlString> objectNames;

	MoveStateContext* moveCtx;

	bool crouch;
};

