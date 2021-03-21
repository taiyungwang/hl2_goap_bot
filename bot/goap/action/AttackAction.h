#pragma once

#include "DestroyObjectAction.h"

class AttackAction: public DestroyObjectAction {
public:
	AttackAction(Blackboard& blackboard);

	bool precondCheck();
private:
	bool targetDestroyed() const;

	edict_t* getTargetedEdict() const;

};
