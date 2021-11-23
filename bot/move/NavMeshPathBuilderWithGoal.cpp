#include "NavMeshPathBuilderWithGoal.h"

#include <player/Bot.h>
#include <nav_mesh/nav_area.h>
#include <IEngineTrace.h>
#include <gametrace.h>

NavMeshPathBuilderWithGoal::NavMeshPathBuilderWithGoal(Bot &self,
		const Vector &goal, float targetRadius) :
		NavMeshPathBuilder(self.getTeam()), self(self), goal(goal), targetRadius(
				targetRadius) {
}

float NavMeshPathBuilderWithGoal::getHeuristicCost(CNavArea *area) const {
	return area->GetCenter().DistTo(goal);
}

bool NavMeshPathBuilderWithGoal::foundGoal(CNavArea *area) {
	Vector loc;
	if (area->Contains(goal)) {
		// goal may be much higher than the area.
		area->GetClosestPointOnArea(goal, &loc);
		return goal.z - loc.z <= HumanHeight;
	}
	if (area->GetCenter().DistTo(goal) < 100.0f) {
		area->GetClosestPointOnArea(goal, &loc);
		CGameTrace tr;
		self.traceMove(tr, loc,
				(goal - loc).Normalized() * (goal.DistTo(loc) - targetRadius)+ loc,
				area->GetAttributes() & NAV_MESH_CROUCH, CTraceFilterHitAll());
		return !tr.DidHit();
	}
	return false;
}
