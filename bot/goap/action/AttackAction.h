#pragma once

#include "DestroyObjectAction.h"

class AttackAction: public DestroyObjectAction {
public:
	AttackAction(Blackboard& blackboard);

	bool precondCheck();

	virtual bool postCondCheck();

private:
	bool isVisible(const Vector& end, edict_t* target) const;

	void getAdjustedTargetPos(Vector& targetLoc,
			WeaponFunction* weapFunc) const;

	edict_t* getTargetedEdict() const;

};
