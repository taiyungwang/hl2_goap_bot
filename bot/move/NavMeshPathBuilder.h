#pragma once

#include <nav_mesh/nav.h>
#include <stack>
#include <utility>

class NavMeshPathBuilder {
public:
	using Path = std::stack<std::pair<int, int>>;

	NavMeshPathBuilder(int team) :
			team(team) {
	}

	virtual ~NavMeshPathBuilder() {
	}

	void build(Path &path, CNavArea *start);

protected:
	virtual float getHeuristicCost(CNavArea *area) const = 0;

	virtual bool foundGoal(CNavArea *area) = 0;

	virtual float getCost(CNavArea *area) const;

	virtual bool shouldSearch(CNavArea *area) const;

private:
	int team = 0;

	void considerNeighbor(CNavArea *from, CNavArea *area,
			NavTraverseType how, float length) const;
};
