#pragma once

#include <goap/action/GoToEntityAction.h>

class DODObjectiveResource;

class CapturePointAction: public GoToEntityAction {
public:

	static void startRound();

	static void endRound();

	CapturePointAction(Blackboard& blackboard);

	bool execute();

	bool precondCheck() {
		return objectiveResource != nullptr && GoToEntityAction::precondCheck();
	}

private:
	static DODObjectiveResource *objectiveResource;

	static CUtlMap<edict_t*, int> capPoints;

	bool isDepleted() const {
		return !isAvailable(item);
	}

	bool isAvailable(edict_t* ent) const;

	void selectItem(CUtlLinkedList<edict_t*>& active);

};
