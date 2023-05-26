#pragma once

#include <goap/action/GoToEntityWithGivenNameAction.h>

class DODObjectives;
class DODObjective;

class CapturePointAction: public GoToEntityWithGivenNameAction {
public:
	CapturePointAction(Blackboard& blackboard);

	virtual bool precondCheck() override;

	virtual bool execute();

	void setObjectives(const DODObjectives* objectives) {
		this->objectives = objectives;
	}

protected:
	virtual bool isAvailable(const DODObjective& obj);

	virtual bool isAvailable(edict_t* ent);

	void selectFromActive(CUtlLinkedList<edict_t*>& active) {
		item = randomChoice(active);
	}

	const DODObjectives* objectives = nullptr;
};
