#pragma once

#include "CapturePointAction.h"

class DODDefuseBombAction : public CapturePointAction {
public:
	DODDefuseBombAction(Bot *self);

	bool precondCheck() override;

	virtual bool execute();

	virtual bool isInterruptable() const {
		return interruptable;
	}

	float getCost() {
		return GoToAction::getCost() + 2.0f;
	}

protected:
	bool interruptable = true;

	virtual bool isAvailable(const DODObjective& obj);

	virtual bool isActingOnBomb(edict_t* teammate) const;

private:
	bool isAvailable(edict_t* ent);
};
