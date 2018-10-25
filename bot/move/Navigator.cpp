#include "Navigator.h"

#include <move/MoveStateContext.h>
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <player/Player.h>
#include <weapon/Weapon.h>
#include <navmesh/nav_mesh.h>
#include <navmesh/nav_pathfind.h>
#include <util/UtilTrace.h>
#include <eiface.h>
#include <ivdebugoverlay.h>
#include <bspflags.h>
#include <in_buttons.h>

extern CNavMesh* TheNavMesh;

Navigator::Navigator(Blackboard& blackboard) :
		blackboard(blackboard) {
	moveCtx = new MoveStateContext(blackboard);
}

Navigator::~Navigator() {
	delete moveCtx;
}

bool Navigator::step() {
	if (!checkCanMove()) {
		return false;
	}
	CNavArea* currentArea = getCurrentArea(blackboard.getSelf()->getCurrentPosition());
	if (path->Count() > 0
			&& currentArea != path->Top() && moveCtx->isStuck()) {
		if (!buildPath(finalGoal, *path)) {
			// repath failed.
			return true;
		}
		moveCtx->setStuck(false);
	}
	getNextArea();
	NavAttributeType meshType =
			path->Count() < 1 ?
					NAV_MESH_INVALID :
					static_cast<NavAttributeType>(path->Top()->GetAttributes());
	moveCtx->move(meshType);
	if (blackboard.getBlocker() == nullptr
			&& blackboard.getTargetedPlayer() == nullptr
			&& !moveCtx->nextGoalIsLadderStart() && !blackboard.isOnLadder()) {
		// look at the farthest visible area.
		const Player* self = blackboard.getSelf();
		Vector eyePos = self->getEyesPos();
		float eyeHt = eyePos.DistTo(self->getCurrentPosition());
		for (int i = 0; i < path->Count(); i++) {
			Vector targetView = path->Element(i)->GetCenter();
			targetView.z += eyeHt;
			trace_t result;
			UTIL_TraceHull(eyePos, targetView, Vector(0.0f, -1.0f, -1.0f),
					Vector(0.0f, 1.0f, 1.0f), MASK_SHOT | MASK_VISIBLE,
					FilterSelfAndTarget(self->getEdict()->GetIServerEntity(),
							nullptr), &result);
			if (result.fraction >= 1.0f) {
				blackboard.setViewTarget(targetView);
			}
		}
	}
	return moveCtx->isStuck()
			|| (!moveCtx->hasGoal() && (reachedGoal() || !moveCtx->reachedGoal()));
}

void Navigator::start(CUtlStack<CNavArea*>* path, const Vector& goal, float targetRadius) {
	finalGoal = goal;
	this->targetRadius = targetRadius;
	this->path = path;
	moveCtx->stop();
	if (path->Count() > 0) {
		moveCtx->setGoal(path->Top()->GetCenter());
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

bool Navigator::isConnectionOnFloor(const CNavArea* from, const CNavArea* to) {
	if (from == nullptr) {
		return false;
	}
	for (int i = 0; i < NUM_DIRECTIONS; i++) {
		const NavConnectVector* connections = from->GetAdjacentAreas(
				static_cast<NavDirType>(i));
		FOR_EACH_VEC(*connections, j)
		{
			if (connections->Element(j).area == to) {
				return true;
			}
		}
	}
	return false;
}

CNavArea* Navigator::getArea(edict_t* ent) {
	CNavArea* area = TheNavMesh->GetNavArea(ent, GETNAVAREA_CHECK_GROUND);
	return area == nullptr ? TheNavMesh->GetNearestNavArea(ent) : area;
}

CNavArea* Navigator::getCurrentArea(const Vector& pos) {
	CNavArea* startArea = TheNavMesh->GetNavArea(pos);
	if (startArea == nullptr || startArea->GetCenter().z - pos.z > JumpHeight) {
		startArea = TheNavMesh->GetNearestNavArea(pos, 120.0f);
	}
	return startArea;
}

bool Navigator::buildPath(const Vector& targetLoc, CUtlStack<CNavArea*>& path) const {
	const Player* self = blackboard.getSelf();
	CNavArea* area = TheNavMesh->GetNavArea(targetLoc);
	if (area == nullptr) {
		area = TheNavMesh->GetNearestNavArea(targetLoc, 1000.0f, false, false);
		if (area == nullptr) {
			return false;
		}
	}
	path.Clear();
	path.Push(area);
	CNavArea* startArea = getArea(self->getEdict());
	if (!NavAreaBuildPath(startArea, path.Top(), nullptr,
			ShortestPathCost(self->getTeam()))) {
		path.Top()->Draw();
		Warning("Could not reach area %d.\n", path.Top()->GetID());
		path.Pop();
		return false;
	}
	for (CNavArea* area = path.Top()->GetParent();
			area != startArea && area != nullptr; area = area->GetParent()) {
		path.Push(area);
	}
	return true;
}

void Navigator::getNextArea() {
	CNavArea* startArea = getCurrentArea(
			blackboard.getSelf()->getCurrentPosition());
	bool canGetNextArea = path->Count() > 0 && !moveCtx->nextGoalIsLadderStart()
			&& startArea == path->Top();
	if (canGetNextArea && path->Count() > 2) {
		int nextAreaAttr = path->Element(1)->GetAttributes();
		canGetNextArea = moveCtx->hasGoal() || !(nextAreaAttr & NAV_MESH_JUMP)
				|| !(nextAreaAttr & NAV_MESH_CROUCH)
				|| !(nextAreaAttr & NAV_MESH_PRECISE);
	}
	if (canGetNextArea) {
		path->Pop();
		if (path->Count() == 0) {
			const Vector& loc = blackboard.getSelf()->getCurrentPosition();
			const Vector& dir = finalGoal - loc;
			float dist = dir.Length() - targetRadius;
			if (dist > 0.0f) {
				Vector dest = dir.Normalized() * dist + loc;
				// check for cases where the end position is against a wall.
				float halfHull = 17.0f;
				// check twice for cases where an item is placed near a corner.
				for (int i = 0; i < 2; i++) {
					trace_t result;
					Vector traceEnd = dest + dir.Normalized() * halfHull;
					UTIL_TraceHull(dest, traceEnd,
							Vector(0.0f, -halfHull, 0.0f),
							Vector(0.0f, halfHull, HumanHeight),
							MASK_NPCSOLID_BRUSHONLY, nullptr,
							COLLISION_GROUP_NONE, &result);
					if (result.fraction >= 1.0f) {
						break;
					}
					dest += result.plane.normal
							* (traceEnd - result.endpos).Length();
				}
				moveCtx->setGoal(dest);
				finalGoal = dest;
			}
		}
	}
	extern ConVar mybot_debug;
	if (mybot_debug.GetBool()) {
		extern IVDebugOverlay *debugoverlay;
		debugoverlay->AddLineOverlay(blackboard.getSelf()->getEyesPos(),
				finalGoal, 255, 255, 255, true,
				NDEBUG_PERSIST_TILL_NEXT_SERVER);
	}
	if (path->Count() > 0) {
		if (mybot_debug.GetBool() && path->Count() > 0) {
			path->Top()->Draw();
		}
		if ((moveCtx->nextGoalIsLadderStart() || blackboard.isOnLadder()
				|| isConnectionOnFloor(startArea, path->Top())
				|| (!findLadder(startArea, path->Top(), CNavLadder::LADDER_UP)
						&& !findLadder(startArea, path->Top(),
								CNavLadder::LADDER_DOWN)))
				&& !moveCtx->nextGoalIsLadderStart()) {
			moveCtx->setGoal(path->Top()->GetCenter());
		}
	}
}


bool Navigator::reachedGoal() const {
	return path->Count() == 0
				&& (blackboard.getSelf()->getCurrentPosition()
				- finalGoal).AsVector2D().Length()
				< targetRadius + MoveStateContext::TARGET_OFFSET;
}

bool Navigator::findLadder(const CNavArea* from, const CNavArea* to,
		CNavLadder::LadderDirectionType dir) {
	if (from == nullptr || to == nullptr) {
		return nullptr;
	}
	const NavLadderConnectVector* laddersFrom = from->GetLadders(dir);
	FOR_EACH_VEC(*laddersFrom, i)
	{
		const CNavLadder* ladder = laddersFrom->Element(i).ladder;
		if (ladder->IsConnected(to, dir)) {
			const Vector* start = &ladder->m_top, *end = &ladder->m_bottom;
			if (dir == CNavLadder::LADDER_UP) {
				start = &ladder->m_bottom;
				end = &ladder->m_top;
			}
			moveCtx->setGoal(*start);
			moveCtx->setLadderEnd(*end);
			moveCtx->setLadderDir(dir);
			return true;
		}
	}
	return false;
}
