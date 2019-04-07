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

	static CUtlMap<edict_t*, int> ctrlPoints;

	static CUtlVector<CCopyableUtlVector<edict_t*>> capTarget;

	virtual bool isAvailable(int idx) const;

	virtual bool isAvailable(edict_t* ent) const;

	virtual bool findTargetLoc();

private:
	static void addCapTarget(const Vector& pos, const CUtlLinkedList<edict_t*>& targets);

	void selectFromActive(CUtlLinkedList<edict_t*>& active);

};
