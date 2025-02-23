#pragma once

#include <nav_mesh/nav.h>
#include <vector>
#include <utility>

/**
 * A more flexible A* path finder when compared to the version offered by the SDK.
 */
class NavMeshPathBuilder {
public:
	using Path = std::vector<std::pair<int, int>>;

	NavMeshPathBuilder(int team) :
			team(team) {
	}

	virtual ~NavMeshPathBuilder() {
	}

	/**
	 * Builds the path from the given area to the goal area.
	 */
	void build(Path &path, CNavArea *start);

protected:
	virtual float getHeuristicCost(CNavArea *area) const = 0;

	/**
	 * @return True to successfully terminate the search.
	 */
	virtual bool foundGoal(CNavArea *area) = 0;

	virtual float getCost(CNavArea *area) const;

	virtual bool shouldSearch(CNavArea *area) const;

private:
	int team = 0;

	void considerNeighbor(CNavArea *from, CNavArea *area,
			NavTraverseType how, float length) const;
};
