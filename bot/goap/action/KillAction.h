#pragma once

#include "AttackAction.h"

class KillAction: public AttackAction {
public:
	KillAction(Blackboard& blackboard);

	bool precondCheck();

	bool execute() override;

private:
	int framesToWait = 0;

	int target = 0;

	bool targetDestroyed() const;

	edict_t* getTargetedEdict() const;
};
