#include "SnipeAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>
#include <navmesh/nav_mesh.h>
#include <util/UtilTrace.h>
#include <vstdlib/random.h>

SnipeAction::SnipeAction(Blackboard& blackboard) : GoToAction(blackboard) {
	effects = {WorldProp::ENEMY_SIGHTED, true};
}

bool SnipeAction::precondCheck() {
	path.Clear();
	if (RandomInt(0, 1) == 0) {
		return false;
	}
	extern CNavMesh* TheNavMesh;
	NavAreaCollector collector;
	float maxDist = 5000.0f;
	targetLoc = blackboard.getSelf()->getCurrentPosition();
	if (!TheNavMesh->ForAllAreasInRadius(collector, targetLoc, maxDist)) {
		return false;
	}
	CUtlVector<CNavArea*> hideAreas;
	FOR_EACH_VEC(collector.m_area, i)
	{
		if (!(collector.m_area[i]->GetAttributes() & NAV_MESH_DONT_HIDE)
				&& collector.m_area[i]->GetHidingSpots()->Count() > 0) {
			hideAreas.AddToTail(collector.m_area[i]);
		}
	}
	if (hideAreas.IsEmpty()) {
		return false;
	}
	do {
		int area = RandomInt(0, hideAreas.Count() - 1);
		auto* hidingSpots = hideAreas[area]->GetHidingSpots();
		int hideSpot = -1;
		for (int i = 0; i < hidingSpots->Count(); i++) {
			auto* spot = hidingSpots->Element(i);
			hideSpot = i;
			targetLoc = hidingSpots->Element(i)->GetPosition();
			break;
		}
		if (hideSpot > -1) {
			GoToAction::precondCheck();
		}
		hideAreas.Remove(area);
	} while (hideAreas.Count() > 0 && path.Count() <= 0);
	if (path.Count() == 0) {
		return false;
	}
	Vector pos = targetLoc;
	pos.z += HumanHeight;
	float furthest = 0.0f;
	for (float currFacing = 0.0f; currFacing < 360.0f; currFacing += 20.0f) {
		QAngle angle(0.0f, currFacing, 0.0f);
		Vector aim;
		AngleVectors(angle, &aim);
		trace_t result;
		UTIL_TraceHull(pos, pos + aim * 2000.0f, Vector(0.0f, -1.0f, -1.0f),
				Vector(0.0f, 1.0f, 1.0f), MASK_SHOT | MASK_VISIBLE,
				FilterSelfAndTarget(blackboard.getSelf()->getEdict()->GetIServerEntity(),
						nullptr), &result);
		if (result.fraction >= 1.0f) {
			facing = currFacing;
			break;
		}
		float dist = result.endpos.DistTo(result.startpos);
		if (dist > furthest) {
			furthest = dist;
			facing = currFacing;
		}
	}
	duration = 300;
	return true;
}

bool SnipeAction::execute() {
	if (!GoToAction::execute()) {
		return false;
	}
	if (!GoToAction::postCondCheck()) {
		return true;
	}
	QAngle angle(0.0f, facing, 0.0f);
	Vector aim;
	AngleVectors(angle, &aim);
	blackboard.setViewTarget(aim);
	return --duration <= 0;
}
