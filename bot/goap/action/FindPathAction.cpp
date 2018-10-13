#include "FindPathAction.h"

#include <player/Player.h>
#include <player/Blackboard.h>
#include <navmesh/nav_mesh.h>
#include <navmesh/nav_pathfind.h>

extern CNavMesh* TheNavMesh;

CNavArea* FindPathAction::getArea(edict_t* ent) {
	CNavArea* area = TheNavMesh->GetNavArea(ent, GETNAVAREA_CHECK_GROUND);
	return area == nullptr ? TheNavMesh->GetNearestNavArea(ent) : area;
}

void FindPathAction::build() {
	const Player* self = blackboard.getSelf();
	CNavArea* area = TheNavMesh->GetNavArea(targetLoc);
	if (area == nullptr) {
		area = TheNavMesh->GetNearestNavArea(targetLoc, 1000.0f, false, false);
		if (area == nullptr) {
			return;
		}
	}
	path.Clear();
	path.Push(area);
	CNavArea* startArea = getArea(self->getEdict());
	if (!NavAreaBuildPath(startArea, path.Top(), nullptr,
			ShortestPathCost(self->getTeam()))) {
		Warning("Could not reach area %d.\n", path.Top()->GetID());
		path.Pop();
		return;
	}
	for (CNavArea* area = path.Top()->GetParent();
			area != startArea && area != nullptr; area = area->GetParent()) {
		path.Push(area);
	}
}

void FindPathAction::init() {
	blackboard.setTargetLocation(targetLoc);
	blackboard.setPath(&path);
}
