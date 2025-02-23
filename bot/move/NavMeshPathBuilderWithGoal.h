#pragma once

#include "NavMeshPathBuilder.h"

/**
 * A * path search with a specific area given as the goal.
 */
class NavMeshPathBuilderWithGoal: public NavMeshPathBuilder {
public:
	NavMeshPathBuilderWithGoal(int team, CNavArea *goal) :
			NavMeshPathBuilder(team), goal(goal) {
	}

private:
	CNavArea *goal;

	float getHeuristicCost(CNavArea *area) const override;

	bool foundGoal(CNavArea *area) override {
		return area == goal;
	}
};
