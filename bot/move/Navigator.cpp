#include "Navigator.h"

#include <move/MoveStateContext.h>
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <player/Bot.h>
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

extern ConVar mybot_debug;

extern IVDebugOverlay *debugoverlay;

Navigator::Navigator(Blackboard& blackboard) :
		blackboard(blackboard) {
	moveCtx = new MoveStateContext(blackboard);
}

Navigator::~Navigator() {
	delete moveCtx;
}

bool Navigator::step() {
	if (moveCtx->isStuck() || reachedGoal()) {
		return true;
	}
	if (!checkCanMove()) {
		return false;
	}
	getNextArea();
	const Player* self = blackboard.getSelf();
	Vector loc = self->getCurrentPosition();
	CNavArea* area = getCurrentArea(loc);
	int attributes = area == nullptr ? NAV_MESH_INVALID: area->GetAttributes();
	if (path->Count() == 0) {
		moveCtx->setTargetOffset(targetRadius);
		moveCtx->setGoal(finalGoal);
		moveCtx->traceMove();
	} else if (lastArea != nullptr) {
		attributes = path->Top()->GetAttributes();
		if (mybot_debug.GetBool()) {
			path->Top()->Draw();
		}
		Vector lastAreaEnd;
		if (moveCtx->nextGoalIsLadderStart()) {
			if (blackboard.isOnLadder()) {
				moveCtx->setGoal(path->Top()->GetCenter());
			}
		} else if (getPortal(lastAreaEnd, lastArea, path->Top())) {
			Vector topAreaStart;
			path->Top()->GetClosestPointOnArea(lastAreaEnd, &topAreaStart);
			if (lastArea->GetAttributes() & NAV_MESH_PRECISE) {
				attributes = lastArea->GetAttributes();
			}
			if ((area != lastArea && ((attributes & NAV_MESH_JUMP)
					|| (attributes & NAV_MESH_PRECISE))) || !canMoveTo(topAreaStart)) {
				topAreaStart = lastArea->GetCenter();
				moveCtx->trace(topAreaStart);
			}
			// TODO: magic number.  Do we need some sort of logic to see if this is valid?
			if (((attributes & NAV_MESH_JUMP) ||
					((attributes & NAV_MESH_CROUCH) && !(area->GetAttributes() & NAV_MESH_CROUCH)))
					&& lastAreaEnd.AsVector2D().DistTo(loc.AsVector2D()) > 40.0f) {
				attributes = NAV_MESH_INVALID;
			}
			moveCtx->setGoal(topAreaStart);
		} else if (!findLadder(lastArea, path->Top())) {
			Warning("Unable to find next goal.\n");
		}
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
	buildPathStartArea = nullptr;
	this->targetRadius = targetRadius;
	this->path = path;
	lastArea = nullptr;
	moveCtx->stop();
	if (path->Count() == 0) {
		Msg("No path available.\n");
	}
}

bool Navigator::checkCanMove() {
	Weapon* weapon = blackboard.getArmory().getCurrWeapon();
	if (weapon != nullptr && weapon->isDeployed()) {
		blackboard.getButtons().tap(IN_ATTACK2);
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

CNavArea* Navigator::getArea(edict_t* ent) {
	CNavArea* area = TheNavMesh->GetNavArea(ent->GetCollideable()->GetCollisionOrigin());
	return area == nullptr ? TheNavMesh->GetNearestNavArea(ent) : area;
}

CNavArea* Navigator::getCurrentArea(const Vector& pos) const {
	return TheNavMesh->GetNearestNavArea(pos, 10000.0f, false, true, blackboard.getSelf()->getTeam());
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
		Warning("Unable to find area for location.\n");
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
		if (this->moveCtx->trace(goal).DidHit()) {
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

bool Navigator::canMoveTo(Vector goal) const {
	const Vector& loc = blackboard.getSelf()->getCurrentPosition();
	if (path->Count() == 1) {
		goal = (goal - loc).Normalized() * (goal.DistTo(loc) - targetRadius) + loc;
	}
	return !moveCtx->trace(goal).DidHit();
}

void Navigator::getNextArea() {
	const Bot* self = blackboard.getSelf();
	if (mybot_debug.GetBool()) {
		debugoverlay->AddLineOverlay(self->getEyesPos(),
				finalGoal, 255, 255, 255, true,
				NDEBUG_PERSIST_TILL_NEXT_SERVER);
	}
	if (lastArea != nullptr && (path->Count() == 0 || blackboard.isOnLadder()
			|| moveCtx->nextGoalIsLadderStart())) {
		return;
	}
	const Vector& loc = self->getCurrentPosition();
	if (lastArea == nullptr || (lastArea == getCurrentArea(loc)
			&& !(lastArea->GetAttributes() & NAV_MESH_PRECISE)
			&& !(path->Top()->GetAttributes() & NAV_MESH_JUMP))
			|| !moveCtx->hasGoal()) {
		path->Pop(lastArea);
		return;
	}
	int nextAreaAttr = path->Top()->GetAttributes();
	Vector goal;
	if (!getPortal(goal, lastArea, path->Top())
			|| (nextAreaAttr & NAV_MESH_JUMP)
					|| (nextAreaAttr & NAV_MESH_CROUCH)
					|| (nextAreaAttr & NAV_MESH_PRECISE)
					|| !canMoveTo(goal)) {
		return;
	}
	if (path->Count() > 1) {
		CNavArea* nextArea = path->Element(path->Count() - 2);
		if ((nextArea->GetAttributes() & NAV_MESH_JUMP)
				// don't skip to an area that is too far below.
				|| loc.z - nextArea->GetCenter().z > 100.0f
				|| !getPortal(goal, path->Top(), nextArea)
				|| !canMoveTo(goal)) {
			return;
		}
	} else if (!canMoveTo(finalGoal)) {
		return;
	}
	path->Pop(lastArea);
}

bool Navigator::reachedGoal() const {
	return path->Count() == 0 && moveCtx->reachedGoal(targetRadius);
}

bool Navigator::findLadder(const CNavArea* from, const CNavArea* to) {
	if (from == nullptr || to == nullptr) {
		return false;
	}
	for (int i = 0; i < CNavLadder::NUM_LADDER_DIRECTIONS; i++) {
		CNavLadder::LadderDirectionType dir = static_cast<CNavLadder::LadderDirectionType>(i);
		const NavLadderConnectVector* laddersFrom = from->GetLadders(dir);
		FOR_EACH_VEC(*laddersFrom, i)
		{
			const CNavLadder* ladder = laddersFrom->Element(i).ladder;
			if (ladder->IsConnected(to, dir)) {
				const Vector* start = &ladder->m_top;
				Vector end = ladder->m_bottom;
				if (dir == CNavLadder::LADDER_UP) {
					// normalize ladder height when climbing up.
					start = &ladder->m_bottom;
					// assume that the z of the next area can be stepped on.
					to->GetClosestPointOnArea(*start, &end);
					end.x = ladder->m_top.x;
					end.y = ladder->m_top.y;
					// the top the ladder is guaranteed to be above human height
					end.z += 3.0f * HumanHeight;
				}
				float delta = end.z - blackboard.getSelf()->getCurrentPosition().z;
				if (delta == 0.0f || (delta < 0.0f && delta >= -StepHeight)
						|| (delta > 0.0f && delta <= HumanHeight + MoveStateContext::TARGET_OFFSET)) {
					// bot already got off the ladder
					return false;
				}
				moveCtx->setGoal(*start);
				moveCtx->setLadderEnd(end);
				moveCtx->setLadderDir(dir);
				return true;
			}
		}
	}
	return false;
}

bool Navigator::getPortal(Vector& portal, const CNavArea* from, const CNavArea* to) {
	Vector toCenter = to->GetCenter();
	for (int i = 0; i < NUM_DIRECTIONS; i++) {
		NavDirType dir = static_cast<NavDirType>(i);
		const NavConnectVector* connections = from->GetAdjacentAreas(dir);
		FOR_EACH_VEC(*connections, j)
		{
			if (connections->Element(j).area == to) {
				float halfWidth;
				from->ComputePortal(to, dir, &portal, &halfWidth);
				return true;
			}
		}
	}
	return false;
}
