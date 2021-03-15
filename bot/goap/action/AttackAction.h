#pragma once

#include "DestroyObjectAction.h"

class AttackAction: public DestroyObjectAction {
public:
	AttackAction(Blackboard& blackboard);

	bool precondCheck();

	virtual bool goalComplete();

private:

	edict_t* getTargetedEdict() const;

};
