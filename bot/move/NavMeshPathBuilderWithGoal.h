#pragma once

#include "NavMeshPathBuilder.h"

class Bot;

class NavMeshPathBuilderWithGoal: public NavMeshPathBuilder {
public:
	NavMeshPathBuilderWithGoal(Bot& self,
			const Vector &goal, float targetRadius);

private:
	float targetRadius;

	const Vector &goal;

	Bot& self;

	float getHeuristicCost(CNavArea *area) const override;

	bool foundGoal(CNavArea *area) override;
};
