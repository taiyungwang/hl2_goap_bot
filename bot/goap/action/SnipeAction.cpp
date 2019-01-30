#include "SnipeAction.h"

#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <player/Bot.h>
#include <weapon/Weapon.h>
#include <weapon/Deployer.h>
#include <navmesh/nav_mesh.h>
#include <util/UtilTrace.h>
#include <vstdlib/random.h>
#include <in_buttons.h>

SnipeAction::SnipeAction(Blackboard& blackboard) : GoToAction(blackboard) {
	effects = {WorldProp::ENEMY_SIGHTED, true};
}

bool SnipeAction::precondCheck() {
	deployed = false;
	path.Clear();
	if (RandomInt(0, 1) == 0) {
		return false;
	}
	extern CNavMesh* TheNavMesh;
	NavAreaCollector collector;
	targetLoc = blackboard.getSelf()->getCurrentPosition();
	if (!TheNavMesh->ForAllAreasInRadius(collector, targetLoc, 5000.0f)) {
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
	pos.z += HumanCrouchHeight;
	float furthest = 0.0f;
	for (float currFacing = -180.0f; currFacing < 180.0f; currFacing += 20.0f) {
		QAngle angle(0.0f, currFacing, 0.0f);
		Vector aim;
		AngleVectors(angle, &aim);
		trace_t result;
		CTraceFilterWorldAndPropsOnly filter;
		UTIL_TraceLine(pos, pos + aim * 2000.0f,
				CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_PLAYERCLIP,
				NULL, COLLISION_GROUP_NONE, &result);
		if (!result.DidHit() && !result.startsolid) {
			furthest = 4000000.0f;
			facing = currFacing;
			break;
		}
		float dist = result.endpos.DistToSqr(pos);
		if (dist > furthest) {
			furthest = dist;
			facing = currFacing;
		}
	}
	if (furthest < 1000000.0) {
		return false;
	}
	duration = 300;
	return true;
}

bool SnipeAction::execute() {
	if (!GoToAction::postCondCheck() && !GoToAction::execute()) {
		if (path.Count() == 1) {
			QAngle angle(0.0f, facing, 0.0f);
			Vector aim;
			AngleVectors(angle, &aim);
			blackboard.setViewTarget(aim);
		}
		return false;
	}
	if (!GoToAction::postCondCheck()) {
		return true;
	}
	Deployer* deployer = blackboard.getArmory().getCurrWeapon()->getDeployer();
	if (deployer == nullptr) {
		blackboard.getButtons().hold(IN_DUCK);
		deployed = true;
	} else if (!deployed && blackboard.getAimAccuracy(blackboard.getViewTarget()) > 0.9f) {
		deployed = deployer->execute(blackboard);
	}
	blackboard.lookStraight();
	return deployed && --duration <= 0;
}
