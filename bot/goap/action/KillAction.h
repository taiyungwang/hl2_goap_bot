#pragma once

#include "AttackAction.h"

class KillAction: public AttackAction {
public:
	KillAction(Blackboard& blackboard);

	bool precondCheck();
private:
	bool targetDestroyed() const;

	edict_t* getTargetedEdict() const;

};
