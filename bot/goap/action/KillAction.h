#pragma once

#include "AttackAction.h"

class KillAction: public AttackAction {
public:
	KillAction(Blackboard& blackboard);

	bool precondCheck();

	virtual bool execute() override {
		return --framesToWait > 0 ? false : AttackAction::execute();
	}

private:
	int framesToWait = 0;

	int target = 0;

	bool targetDestroyed() const;

	edict_t* getTargetedEdict() const;
};
