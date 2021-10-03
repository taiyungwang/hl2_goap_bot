#include "Navigator.h"

#include <move/MoveStateContext.h>
#include <move/MoveLadder.h>
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <player/Bot.h>
#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>
#include <nav_mesh/nav_mesh.h>
#include <nav_mesh/nav_pathfind.h>
#include <util/UtilTrace.h>
#include <util/BasePlayer.h>
#include <eiface.h>
#include <ivdebugoverlay.h>
#include <bspflags.h>
#include <in_buttons.h>

extern CNavMesh* TheNavMesh;

extern IVDebugOverlay *debugoverlay;

extern ConVar mybot_debug;

static ConVar maxAreaTime("my_bot_max_area_time", "70");

static ConVar mybotStuckThreshold("mybot_stuck_threshold", "80.0f");

Navigator::Navigator(Blackboard& blackboard) :
		blackboard(blackboard) {
	moveCtx = new MoveStateContext(blackboard);
}

Navigator::~Navigator() {
	delete moveCtx;
}

bool Navigator::step() {
	if (moveCtx->isStuck() || reachedGoal() || blackboard.getBlocker() != nullptr) {
		return true;
	}
	if (!checkCanMove()) {
		return false;
	}
	const Player* self = blackboard.getSelf();
	Vector loc = self->getCurrentPosition();
	CNavArea* area = getCurrentArea(loc);
	if (area == nullptr) {
		return true;
	}
	lastArea = area;
	Vector goal(moveCtx->getGoal());
	int attributes = nextArea == nullptr ? area->GetAttributes() : nextArea->GetAttributes();
	bool crouching = attributes & NAV_MESH_CROUCH;
	if (path->Count() > 0 && !(blackboard.isOnLadder() && moveCtx->hasGoal())
			&& getNextArea(goal, loc, area)) {
		if (!moveCtx->nextGoalIsLadderStart()) {
			moveCtx->setGoal(goal);
		}
	} else if (!moveCtx->hasGoal()) {
		touchedAreaCenter = touchedAreaCenter || loc.DistTo(nextArea->GetCenter()) < HalfHumanWidth
					|| moveCtx->isAtTarget(nextArea->GetCenter(), moveCtx->getTargetOffset());
		bool hasPortalToNext = getPortalToNextArea(goal);
		if (path->Count() == 0 && area == nextArea) {
			nextArea->GetClosestPointOnArea(finalGoal, &goal);
			hasPortalToNext = true;
		}
		if (!touchedAreaCenter) {
			if ((!hasPortalToNext || (attributes & NAV_MESH_PRECISE)
				|| (path->Count() > 0 && ((path->Top()->GetAttributes() & NAV_MESH_JUMP)))
				|| !canMoveTo(goal, crouching))) {
				goal = nextArea->GetCenter();
			}
		} else if (!hasPortalToNext && path->Count() > 0) {
			setLadderStart();
		}
		if (path->Count() == 0 && moveCtx->getGoal() != finalGoal
				&& ((touchedAreaCenter && finalGoal.AsVector2D().DistTo(goal.AsVector2D ())
						<= targetRadius)
					|| canMoveTo(finalGoal, crouching)
					|| moveCtx->reachedGoal(moveCtx->getTargetOffset()))) {
			moveCtx->setTargetOffset(targetRadius);
			goal = finalGoal;
		}
		if (!moveCtx->nextGoalIsLadderStart()) {
			moveCtx->setGoal(goal);
		}
	}
	if (mybot_debug.GetBool()) {
		nextArea->Draw();
	}
	// magic number from https://developer.valvesoftware.com/wiki/Dimensions#Horizontal_.28To_Equal_Height.29
	if (((attributes & NAV_MESH_JUMP) ||
			((attributes & NAV_MESH_CROUCH) && !(area->GetAttributes() & NAV_MESH_CROUCH)))
			&& moveCtx->getGoal().AsVector2D().DistTo(loc.AsVector2D()) > 136.0f) {
		attributes = NAV_MESH_INVALID;
	}
	if (area == lastArea) {
		areaTime++;
	} else {
		areaTime = 0;
	}
	int maxTime = maxAreaTime.GetInt();
	if ((attributes & NAV_MESH_CROUCH) || moveCtx->nextGoalIsLadderStart()) {
		maxTime *= 1.5f;
	}
	if (areaTime > maxTime
			&& BasePlayer(self->getEdict()).getVelocity().Length() < mybotStuckThreshold.GetFloat()) {
		areaTime = 0;
		moveCtx->setStuck(true);
	}
	float goalDist = moveCtx->getGoal().DistTo(loc);
	if (goalDist > 1000.0f) {
		Warning("Goal is too far from current position: %f.\n", goalDist);
		return true;
	}
	moveCtx->move(attributes);
	if (mybot_debug.GetBool()) {
		debugoverlay->AddLineOverlay(loc,
				moveCtx->getGoal(), 0, 255, 255, true,
				NDEBUG_PERSIST_TILL_NEXT_SERVER);
	}
	return false;
}

void Navigator::start(CUtlStack<CNavArea*>* path, const Vector& goal, float targetRadius) {
	finalGoal = goal;
	lastArea = nextArea = buildPathStartArea = nullptr;
	this->targetRadius = targetRadius;
	this->path = path;
	moveCtx->setGoal(blackboard.getSelf()->getCurrentPosition());
	moveCtx->stop();
	if (path->Count() == 0) {
		Msg("No path available.\n");
	}
}

bool Navigator::checkCanMove() {
	Weapon* weapon = blackboard.getSelf()->getArsenal().getCurrWeapon();
	if (weapon != nullptr && weapon->isDeployed()) {
		weapon->undeploy(blackboard);
		return false;
	}
	return true;
}

NavDirType getConnectedDir(const CNavArea* from, const CNavArea* to) {
	if (from == nullptr) {
		return NUM_DIRECTIONS;
	}
	for (int i = 0; i < NUM_DIRECTIONS; i++) {
		const NavConnectVector* connections = from->GetAdjacentAreas(static_cast<NavDirType>(i));
		FOR_EACH_VEC(*connections, j)
		{
			if (connections->Element(j).area == to) {
				return static_cast<NavDirType>(i);
			}
		}
	}
	return NUM_DIRECTIONS;
}

CNavArea* Navigator::getArea(edict_t* ent, int team) {
	return TheNavMesh->GetNearestNavArea(ent->GetCollideable()->GetCollisionOrigin(),
			10000.0f, false, true, team);
}

CNavArea* Navigator::getCurrentArea(const Vector& pos) const {
	int team = blackboard.getSelf()->getTeam();
	CNavArea* area = TheNavMesh->GetNearestNavArea(pos, 10000.0f, false, true, team);
	if (area == nullptr) {
		area = TheNavMesh->GetNearestNavArea(pos, 10000.0f, false, false, team);
		if (area != nullptr && area->IsBlocked(team)) {
			area = nullptr;
		}
	}
	return area;
}

bool Navigator::buildPath(const Vector& targetLoc, CUtlStack<CNavArea*>& path) {
	path.Clear();
	const Player* self = blackboard.getSelf();
	if (buildPathStartArea == nullptr) {
		buildPathStartArea = getCurrentArea(self->getCurrentPosition());
		if (buildPathStartArea == nullptr) {
			Warning("Unable to get startArea.\n");
			return false;
		}
	}
	CNavArea* goalArea = getCurrentArea(targetLoc);
	if (goalArea == nullptr) {
		buildPathStartArea = nullptr;
		Warning("Unable to find area for goal.\n");
		if (mybot_debug.GetBool()) {
			debugoverlay->AddLineOverlay(self->getEyesPos(),
					targetLoc, 255, 0, 0, true,
					NDEBUG_PERSIST_TILL_NEXT_SERVER);
		}
		return false;
	}
	CNavArea* closest = nullptr;
	if (!NavAreaBuildPath(buildPathStartArea, goalArea, &targetLoc,
			ShortestPathCost(self->getTeam()), &closest, 0.0f, self->getTeam()) && closest == nullptr) {
		if (mybot_debug.GetBool()) {
			debugoverlay->AddLineOverlay(buildPathStartArea->GetCenter(),
					targetLoc, 255, 0, 0, true,
					NDEBUG_PERSIST_TILL_NEXT_SERVER);
		}
		buildPathStartArea = nullptr;
		Warning("Unable to get to goal area %d.\n", goalArea->GetID());
		if (mybot_debug.GetBool()) {
			goalArea->Draw();
		}
		return false;
	}
	static const float MAX_DIST = 300.0f;
	if (goalArea == nullptr) {
		float dist = closest->GetCenter().DistTo(targetLoc);
		if (dist > MAX_DIST) {
			buildPathStartArea = nullptr;
			Warning("Unable to find goal area for location. Closest is %f\n", dist);
			return false;
		}
		goalArea = closest;
	}
	for (CNavArea* area = goalArea; area != nullptr;
			area = area->GetParent()) {
		path.Push(area);
	}
	if (path.Top() != buildPathStartArea) {
		Vector loc = blackboard.getSelf()->getCurrentPosition(), goal;
		path.Top()->GetClosestPointOnArea(loc, &goal);
		if (this->moveCtx->trace(goal, path.Top()->GetAttributes() & NAV_MESH_CROUCH).DidHit()) {
			Warning("Can't get to start area %d, distance is %f.\n", path.Top()->GetID(),
					loc.DistTo(goal));
			if (mybot_debug.GetBool()) {
				path.Top()->Draw();
			}
			buildPathStartArea = nullptr;
			return false;
		}
	}
	buildPathStartArea = goalArea;
	return true;
}

bool Navigator::getNextArea(Vector& goal, const Vector& loc, const CNavArea* area) {
	if (nextArea == nullptr || area == path->Top()
			// if portal to top is same as last goal and we're not moving
			|| (!moveCtx->hasGoal() && getPortalToNextArea(goal) && goal == moveCtx->getGoal())) {
		path->Pop(nextArea);
		touchedAreaCenter = false;
		if (path->Count() > 0 && !getPortalToNextArea(goal)) {
			setLadderStart();
		}
		return true;
	}
	bool crouching = nextArea->GetAttributes() & NAV_MESH_CROUCH;
	if ((nextArea->GetAttributes() & (NAV_MESH_CROUCH | NAV_MESH_JUMP | NAV_MESH_PRECISE))
			|| moveCtx->nextGoalIsLadderStart() || blackboard.isOnLadder()
			// don't skip areas above and below ground height
			|| fabs(nextArea->GetCenter().z - path->Top()->GetCenter().z) > StepHeight
			|| fabs(loc.z - nextArea->GetCenter().z) > StepHeight
			|| !getPortalToNextArea(goal) || !canMoveTo(goal, crouching)) {
		return false;
	}
	touchedAreaCenter = false;
	path->Pop(nextArea);
	return true;
}

bool Navigator::canMoveTo(Vector goal, bool crouch) const {
	const Vector& loc = blackboard.getSelf()->getCurrentPosition();
	if (path->Count() == 1) {
		goal = (goal - loc).Normalized() * (goal.DistTo(loc) - targetRadius) + loc;
	}
	return !moveCtx->trace(goal, crouch).DidHit();
}

bool Navigator::reachedGoal() const {
	return path->Count() == 0 && moveCtx->getGoal() == finalGoal
			&& moveCtx->reachedGoal(targetRadius);
}

void Navigator::setLadderStart() {
	if (path->Count() == 0) {
		return;
	}
	for (int i = 0; i < CNavLadder::NUM_LADDER_DIRECTIONS; i++) {
		CNavLadder::LadderDirectionType dir = static_cast<CNavLadder::LadderDirectionType>(i);
		const NavLadderConnectVector* laddersFrom = nextArea->GetLadders(dir);
		FOR_EACH_VEC(*laddersFrom, i)
		{
			const CNavLadder* ladder = laddersFrom->Element(i).ladder;
			if (ladder->IsConnected(path->Top(), dir)) {
				const Vector* start = &ladder->m_top;
				Vector end = ladder->m_bottom;
				if (dir == CNavLadder::LADDER_UP) {
					// normalize ladder height when climbing up.
					start = &ladder->m_bottom;
					// assume that the z of the next area can be stepped on.
					path->Top()->GetClosestPointOnArea(*start, &end);
					end.x = ladder->m_top.x;
					end.y = ladder->m_top.y;
					// the top the ladder is guaranteed to be above human height
					end.z += 3.0f * HumanHeight;
				}
				float delta = end.z - blackboard.getSelf()->getCurrentPosition().z;
				if (delta == 0.0f || (delta < 0.0f && delta >= -StepHeight)
						|| (delta > 0.0f && delta <= HumanHeight + MoveLadder::TARGET_OFFSET)) {
					// bot already got off the ladder
					Error("No ladder found from area %d to area %d.\n", nextArea->GetID(),
							path->Top()->GetID());
					return;
				}
				moveCtx->setGoal(*start);
				moveCtx->setLadderEnd(end);
				moveCtx->setLadderDir(dir);
				return;
			}
		}
	}
	Error("No ladder found from area %d to area %d.\n", nextArea->GetID(),
			path->Top()->GetID());
}

bool Navigator::getPortalToNextArea(Vector& portal) const {
	if (path->Count() == 0) {
		return false;
	}
	for (int i = 0; i < NUM_DIRECTIONS; i++) {
		NavDirType dir = static_cast<NavDirType>(i);
		const NavConnectVector* connections = nextArea->GetAdjacentAreas(dir);
		FOR_EACH_VEC(*connections, j)
		{
			if (connections->Element(j).area == path->Top()) {
				float halfWidth;
				nextArea->ComputePortal(path->Top(), dir, &portal, &halfWidth);
				return true;
			}
		}
	}
	return false;
}
