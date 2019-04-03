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

protected:
	static bool isDetonationMap;

	static DODObjectiveResource *objectiveResource;

	static CUtlMap<edict_t*, int> capPoints;

	static CUtlVector<CCopyableUtlVector<edict_t*>> bombs;

	virtual bool isAvailable(int idx) const;

	virtual bool isAvailable(edict_t* ent) const;

	virtual bool findTargetLoc() {
		return objectiveResource != nullptr && GoToEntityAction::findTargetLoc();
	}
private:

	void selectItem(CUtlLinkedList<edict_t*>& active);

};
