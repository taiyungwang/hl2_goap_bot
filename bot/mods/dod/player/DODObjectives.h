#pragma once

#include <utlmap.h>
#include <utlvector.h>
#include <utllinkedlist.h>

class DODObjectiveResource;
class DODObjective;
struct edict_t;

class DODObjectives {
public:
	typedef CCopyableUtlVector<edict_t*> CapTarget;

	DODObjectives() {
		endRound();
	}

	~DODObjectives() {
		endRound();
	}

	/**
	 * @param areaEntity Entity for a given bomb or capture area.
	 * @Return -1 if the index is not found otherwise The index for a given capture or bomb area entity.
	 */
	int getIndex(edict_t *areaEntity) const;

	/**
	 * Callback for when a round starts
	 */
	void startRound();

	/**
	 * Callback for when a round ends.
	 */
	void endRound();

	bool roundStarted() const {
		return objectiveResource != nullptr;
	}

	/**
	 * @return True if the current map is a dentonation type map.
	 */
	bool isDetonation() const {
		return detonation;
	}

	const DODObjective* getObjective(edict_t*) const;

	const CUtlLinkedList<edict_t*>& getCtrlPts() const {
		return ctrlPts;
	}

private:
	bool detonation;

	DODObjectiveResource *objectiveResource = nullptr;

	CUtlMap<edict_t*, int> ctrlPointsMap;

	CUtlLinkedList<edict_t*> ctrlPts;

	CUtlVector<DODObjective*> objectives;

	void addCapTarget(const Vector &pos,
			const CUtlLinkedList<edict_t*> &targets);
};
