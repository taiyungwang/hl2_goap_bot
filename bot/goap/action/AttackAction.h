#pragma once

#include "Action.h"
#include <utllinkedlist.h>

class MoveStateContext;
class WeaponFunction;
struct edict_t;

class AttackAction: public Action {
public:
	AttackAction(Blackboard& blackboard);

	virtual ~AttackAction();

	virtual bool precondCheck();

	bool execute();

	bool goalComplete();

	void abort();

protected:

	bool adjustAim = true;

	unsigned int dur = 0;

	virtual edict_t* getTargetedEdict() const;

	virtual bool isBreakable(edict_t* object) const {
		return false;
	}

	virtual bool targetDestroyed() const;

private:
	CUtlLinkedList<CUtlString> objectNames;

	MoveStateContext* moveCtx;
};

