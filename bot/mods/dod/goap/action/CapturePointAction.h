#pragma once

#include <goap/action/GoToConsumableEntityAction.h>
#include <utlvector.h>

class DODObjectives;
class DODObjective;

class CapturePointAction: public GoToConsumableEntityAction {
public:
	CapturePointAction(Blackboard& blackboard);

	virtual bool execute();

	void setObjectives(const DODObjectives* objectives) {
		this->objectives = objectives;
	}

protected:
	virtual bool isAvailable(const DODObjective& obj);

	virtual bool isAvailable(edict_t* ent);

	virtual bool findTargetLoc();

	void selectFromActive(CUtlLinkedList<edict_t*>& active) {
		item = randomChoice(active);
	}

	const DODObjectives* objectives = nullptr;
};
