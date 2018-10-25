#pragma once

#include <goap/action/GoToConsumableEntityAction.h>
#include <utlvector.h>

class DODObjectiveResource;

class CapturePointAction: public GoToConsumableEntityAction {
public:

	static void startRound();

	static void endRound();

	CapturePointAction(Blackboard& blackboard);

	virtual bool execute();

	virtual bool precondCheck() {
		return objectiveResource != nullptr && GoToConsumableEntityAction::precondCheck();
	}

protected:
	static bool isDetonationMap;

	static DODObjectiveResource *objectiveResource;

	static CUtlMap<edict_t*, int> capPoints;

	static CUtlVector<CCopyableUtlVector<edict_t*>> bombs;

	virtual bool isAvailable(int idx) const;

	bool isDepleted() const {
		return !isAvailable(item);
	}

private:
	bool isAvailable(edict_t* ent) const;

	void selectItem(CUtlLinkedList<edict_t*>& active);

};
