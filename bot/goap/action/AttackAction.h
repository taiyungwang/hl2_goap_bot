#pragma once

#include "Action.h"

class MoveStateContext;
struct edict_t;

class AttackAction: public Action {
public:
	explicit AttackAction(Bot *blackboard);

	~AttackAction() override;

	bool precondCheck() override;

	bool execute() override;

	bool goalComplete() override;

	void abort() override;

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

