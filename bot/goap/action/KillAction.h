#pragma once

#include "DestroyObjectAction.h"

class KillAction: public DestroyObjectAction {
public:
	KillAction(Blackboard& blackboard);

	bool precondCheck();
private:
	bool targetDestroyed() const;

	edict_t* getTargetedEdict() const;

};
