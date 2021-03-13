#pragma once

#include <goap/action/GoToConsumableEntityAction.h>
#include <utlvector.h>

class DODObjectives;

class CapturePointAction: public GoToConsumableEntityAction {
public:
	CapturePointAction(Blackboard& blackboard);

	virtual bool execute();

	void setObjectives(const DODObjectives* objectives) {
		this->objectives = objectives;
	}

protected:
	virtual bool isAvailable(int idx);

	virtual bool isAvailable(edict_t* ent);

	virtual bool findTargetLoc();

	virtual void selectFromActive(CUtlLinkedList<edict_t*>& active);

	const DODObjectives* objectives = nullptr;
};
