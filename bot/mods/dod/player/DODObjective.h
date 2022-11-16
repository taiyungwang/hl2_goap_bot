#pragma once

#include <nav_mesh/nav_pathfind.h>
#include <vector>

class DODObjectiveResource;
struct edict_t;

class DODObjective: public ISearchSurroundingAreasFunctor {
public:
	enum class BombState {
		UNAVILABLE,
		AVAILABLE,
		ACTIVE
	};

	DODObjective(unsigned int idx, DODObjectiveResource &rsc) :
		rsc(rsc), idx(idx){
	}

	void addTarget(edict_t* target) {
		targets.push_back(target);
	}

	bool operator()(CNavArea *area, CNavArea *priorArea,
			float travelDistanceSoFar);

	/**
	 * Checks whether any bomb in a given area is in a given state
	 * @param state Bomb state value, which is one of
	 * 0 - unavailable,
	 * 1 - available
	 * 2 - active
	 * @return True if any of the bombs in the area matches the given state
	 */
	bool hasBombTargetInState(BombState state) const;

	/**
	 * @return Team id of owner of position.
	 */
	int getOwner() const;

	bool hasBombs() const;

	const std::vector<edict_t*>& getTargets() const {
		return targets;
	}

	const std::vector<int>& getHideSpots() const {
		return hideSpots;
	}

private:
	DODObjectiveResource &rsc;

	const unsigned idx;

	std::vector<int> hideSpots;

	std::vector<edict_t*> targets;
};
