#include "NavMeshPathBuilderWithGoal.h"

#include <nav_mesh/nav_area.h>

float NavMeshPathBuilderWithGoal::getHeuristicCost(CNavArea *area) const {
	return area->GetCenter().DistTo(goal->GetCenter());
}
