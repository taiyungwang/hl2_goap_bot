#include "Route.h"
#include "MoveStateContext.h"
#include <nav_mesh/nav_mesh.h>
#include <nav_mesh/nav_area.h>
#include <ivdebugoverlay.h>
#include <edict.h>

extern CNavMesh* TheNavMesh;

extern ConVar mybot_debug;

extern IVDebugOverlay *debugoverlay;


CNavArea* Route::getArea(edict_t* ent) {
	CNavArea* area = TheNavMesh->GetNavArea(ent->GetCollideable()->GetCollisionOrigin());
	return area == nullptr ? TheNavMesh->GetNearestNavArea(ent) : area;
}

CNavArea* Route::getCurrentArea(const Vector& pos, int team) {
	CNavArea* startArea = TheNavMesh->GetNavArea(pos);
	if (startArea == nullptr || startArea->GetCenter().z - pos.z > JumpHeight) {
		startArea =TheNavMesh->GetNearestNavArea(pos, 200.0f, false, false, team);
	}
	return startArea;
}

bool Route::step(const Vector &loc, bool isOnLadder) {
	if (lastArea == nullptr || (path.Count() != 0 && !isOnLadder
			&& !moveCtx.nextGoalIsLadderStart())) {
		getNextArea(loc);
	}
	CNavArea *area = getCurrentArea(loc);
	int attributes = area == nullptr ? NAV_MESH_INVALID : area->GetAttributes();
	if (path.Count() == 0) {
		moveCtx.setTargetOffset(goalRadius);
		moveCtx.setGoal(goal);
		moveCtx.traceMove();
	} else if (lastArea != nullptr) {
		attributes = path.Top()->GetAttributes();
		if (mybot_debug.GetBool()) {
			path.Top()->Draw();
		}
		Vector lastAreaEnd;
		if (moveCtx.nextGoalIsLadderStart()) {
			if (isOnLadder) {
				moveCtx.setGoal(path.Top()->GetCenter());
			}
		} else if (getPortal(lastAreaEnd, lastArea, path.Top())) {
			Vector topAreaStart;
			path.Top()->GetClosestPointOnArea(lastAreaEnd, &topAreaStart);
			if (lastArea->GetAttributes() & NAV_MESH_PRECISE) {
				attributes = lastArea->GetAttributes();
			}
			if ((area != lastArea
					&& ((attributes & NAV_MESH_JUMP)
							|| (attributes & NAV_MESH_PRECISE)))
					|| !canMoveTo(loc, topAreaStart)) {
				topAreaStart = lastArea->GetCenter();
				moveCtx.trace(topAreaStart);
			}
			// TODO: magic number.  Do we need some sort of logic to see if this is valid?
			if (((attributes & NAV_MESH_JUMP)
					|| ((attributes & NAV_MESH_CROUCH)
							&& !(area->GetAttributes() & NAV_MESH_CROUCH)))
					&& lastAreaEnd.AsVector2D().DistTo(loc.AsVector2D())
							> 40.0f) {
				attributes = NAV_MESH_INVALID;
			}
			moveCtx.setGoal(topAreaStart);
		} else if (!findLadder(lastArea, path.Top())) {
			Warning("Unable to find next goal.\n");
		}
	}
	moveCtx.move(attributes);
	if (mybot_debug.GetBool()) {
		debugoverlay->AddLineOverlay(loc, moveCtx.getGoal(), 0, 255, 255, true,
				NDEBUG_PERSIST_TILL_NEXT_SERVER);
	}
	return false;
}


bool Route::reachedGoal() const {
	return path.Count() == 0 && moveCtx.reachedGoal(goalRadius);
}

void Route::getNextArea(const Vector& loc) {
	if (lastArea == nullptr || (lastArea == getCurrentArea(loc)
			&& !(lastArea->GetAttributes() & NAV_MESH_PRECISE)
			&& !(path.Top()->GetAttributes() & NAV_MESH_JUMP))
			|| !moveCtx.hasGoal()) {
		path.Pop(lastArea);
		return;
	}
	int nextAreaAttr = path.Top()->GetAttributes();
	Vector nextLoc;
	if ((nextAreaAttr & NAV_MESH_JUMP) || (nextAreaAttr & NAV_MESH_CROUCH)
			|| (nextAreaAttr & NAV_MESH_PRECISE)
			|| !getPortal(nextLoc, lastArea, path.Top())
			|| !canMoveTo(loc, nextLoc)) {
		return;
	}
	if (path.Count() > 1) {
		CNavArea* nextArea = path.Element(path.Count() - 2);
		if ((nextArea->GetAttributes() & NAV_MESH_JUMP)
				// don't skip to an area that is too far below.
				|| loc.z - nextArea->GetCenter().z > 100.0f
				|| !getPortal(nextLoc, path.Top(), nextArea)
				|| !canMoveTo(loc, nextLoc)) {
			return;
		}
	} else if (!canMoveTo(loc, goal)) {
		return;
	}
	path.Pop(lastArea);
}

bool Route::canMoveTo(const Vector& loc, Vector to) const {
	if (path.Count() == 1) {
		to = (to - loc).Normalized() * (to.DistTo(loc) - goalRadius) + loc;
	}
	return !moveCtx.trace(to).DidHit();
}

bool Route::getPortal(Vector& portal, const CNavArea* from, const CNavArea* to) {
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
