#pragma once

#include "Action.h"

class MoveStateContext;
struct edict_t;

class AttackAction: public Action {
public:
	AttackAction(Bot *blackboard);

	virtual ~AttackAction();

	virtual bool precondCheck();

	virtual bool execute() override;

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
	MoveStateContext* moveCtx;
};

