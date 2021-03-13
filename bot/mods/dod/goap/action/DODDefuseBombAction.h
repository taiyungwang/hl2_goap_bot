#pragma once

#include "DODDefendPointAction.h"

class DodPlayer;

class DODDefuseBombAction : public DODDefendPointAction {
public:
	DODDefuseBombAction(Blackboard& blackboard);

	/**
	 * TODO: Hacky, consider refactoring.
	 */
	virtual bool precondCheck() {
		return GoToAction::precondCheck();
	}

	virtual bool execute();

	virtual bool isInterruptable() const {
		return interruptable;
	}

	float getCost() {
		return GoToAction::getCost() + 2.0f;
	}

protected:
	bool interruptable = true;

	virtual bool isAvailable(int idx);

private:
	/**
	 * TODO: hacky, consider refactoring.
	 */
	void selectFromActive(CUtlLinkedList<edict_t*>& active) {
		CapturePointAction::selectFromActive(active);
	}

	bool findTargetLoc();

	bool isAvailable(edict_t* ent);
};
