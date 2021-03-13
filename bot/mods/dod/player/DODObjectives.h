#pragma once

#include <utlmap.h>
#include <utlvector.h>
#include <utllinkedlist.h>

class DODObjectiveResource;
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

	/**
	 * Checks whether any bomb in a given area is in a given state
	 * @param idx Index for the area from getIndex()
	 * @param state Bomb state value, which is one of
	 * 0 - unavailable,
	 * 1 - available
	 * 2 - active
	 * @return True if any of the bombs in the area matches the given state
	 */
	bool isBombInState(int idx, int state) const;

	/**
	 * @param idx Index for the area from getIndex()
	 * @return Team id of owner of position.
	 */
	int getOwner(int idx) const;

	const CapTarget& getCapTarget(int idx) const {
		return capTargets[idx];
	}

	bool hasBombs(int idx) const;

private:
	bool detonation;

	DODObjectiveResource *objectiveResource = nullptr;

	CUtlMap<edict_t*, int> ctrlPoints;

	CUtlVector<CapTarget> capTargets;

	void addCapTarget(const Vector &pos,
			const CUtlLinkedList<edict_t*> &targets);

};
