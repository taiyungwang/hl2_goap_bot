#pragma once

#include "DODDefendPointAction.h"

class DodPlayer;

class DODDefuseBombAction : public DODDefendPointAction {
public:
	DODDefuseBombAction(Blackboard& blackboard);

	virtual bool execute();

	virtual bool isInterruptable() const {
		return interruptable;
	}

	float getCost() {
		return GoToAction::getCost() + 2.0f;
	}

protected:
	bool interruptable = true;

	virtual bool isTeamMateActingOnBomb(DodPlayer& teammate) const;

	virtual bool isAvailable(int idx) const;

private:
	bool findTargetLoc();

	bool isAvailable(edict_t* ent) const;
};
