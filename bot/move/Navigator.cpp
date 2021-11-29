#include "Navigator.h"

#include <move/MoveStateContext.h>
#include <move/MoveLadder.h>
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <player/Bot.h>
#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>
#include <nav_mesh/nav_mesh.h>
#include <nav_mesh/nav_area.h>
#include <util/UtilTrace.h>
#include <util/BasePlayer.h>
#include <eiface.h>
#include <ivdebugoverlay.h>
#include <bspflags.h>
#include <in_buttons.h>

extern CNavMesh* TheNavMesh;

static ConVar maxAreaTime("my_bot_max_area_time", "80");

Navigator::Navigator(Blackboard& blackboard) :
		blackboard(blackboard) {
	moveCtx = new MoveStateContext(blackboard);
}

Navigator::~Navigator() {
	delete moveCtx;
}

void Navigator::start(const Vector &goal, float targetRadius, bool sprint) {
	this->sprint = sprint;
	finalGoal = goal;
	lastAreaId = -1;
	areaTime = 0;
	this->targetRadius = targetRadius;
	moveCtx->stop();
}

bool Navigator::step() {
	if (moveCtx->isStuck() || reachedGoal() || blackboard.getBlocker() != nullptr
			|| (lastAreaId < 0 && path.empty())) {
		return true;
	}
	if (!checkCanMove()) {
		return false;
	}
	const Player* self = blackboard.getSelf();
	Vector loc = self->getCurrentPosition();
	CNavArea *topArea = path.empty() ? nullptr : TheNavMesh->GetNavAreaByID(std::get<0>(path.top()));
	if (canGetNextArea(loc)) {
		setGoalForNextArea(loc);
		if (!path.empty()) {
			topArea = TheNavMesh->GetNavAreaByID(std::get<0>(path.top()));
		}
	} else if (!moveCtx->hasGoal() && topArea != nullptr) {
		moveCtx->setGoal(topArea->GetCenter());
	}
	CNavArea *lastArea = TheNavMesh->GetNavAreaByID(lastAreaId);
	int attributes = lastArea->GetAttributes();
	if (lastArea == nullptr) {
		return true;
	}
	if ((topArea == nullptr && !path.empty()) || lastArea == nullptr
			|| lastArea->IsBlocked(self->getTeam())
			// bot fell off due to bad pathing.
			|| (!moveCtx->nextGoalIsLadderStart() && !blackboard.isOnLadder()
					&& moveCtx->getGoal().z - loc.z > JumpCrouchHeight
					&& moveCtx->getGoal().AsVector2D().DistTo(loc.AsVector2D())
							< HalfHumanWidth)) {
		return true;
	}
	extern ConVar mybot_debug;
	if (mybot_debug.GetBool() && topArea != nullptr) {
		topArea->Draw();
	}
	// magic number from https://developer.valvesoftware.com/wiki/Dimensions#Horizontal_.28To_Equal_Height.29
	if (((attributes & NAV_MESH_JUMP) || (attributes & NAV_MESH_CROUCH))
			&& moveCtx->getGoal().AsVector2D().DistTo(loc.AsVector2D()) > 136.0f) {
		attributes = NAV_MESH_INVALID;
	}
	int maxTime = maxAreaTime.GetInt();
	if ((attributes & NAV_MESH_CROUCH) || moveCtx->nextGoalIsLadderStart()) {
		maxTime *= 1.5f;
	}
	float speed = BasePlayer(self->getEdict()).getVelocity().Length();
	if (!blackboard.isOnLadder() &&
			(((attributes & NAV_MESH_CROUCH) && speed < 63.0f)
			|| ((attributes & NAV_MESH_WALK) && speed < 150.0f)
			|| speed < 190.0f)) {
		areaTime++;
	} else {
		areaTime = 0;
	}
	if (areaTime > maxTime) {
		areaTime = 0;
		moveCtx->setStuck(true);
	}
	float goalDist = moveCtx->getGoal().DistTo(loc);
	if (moveCtx->hasGoal() && goalDist > 1000.0f
			&& !canMoveTo(moveCtx->getGoal(), attributes & NAV_MESH_CROUCH)) {
		Warning("Goal is too far from current position: %f.\n", goalDist);
		return true;
	}
	if (path.empty()) {
		moveCtx->setTargetOffset(targetRadius);
	}
	if (sprint) {
		blackboard.getButtons().hold(IN_SPEED);
	}
	moveCtx->move(attributes);
	if (mybot_debug.GetBool()) {
		extern IVDebugOverlay *debugoverlay;
		debugoverlay->AddLineOverlay(loc,
				moveCtx->getGoal(), 0, 255, 255, true,
				NDEBUG_PERSIST_TILL_NEXT_SERVER);
	}
	return false;
}

bool Navigator::checkCanMove() {
	Weapon* weapon = blackboard.getSelf()->getArsenal().getCurrWeapon();
	if (weapon != nullptr && weapon->isDeployed()) {
		weapon->undeploy(blackboard);
		return false;
	}
	return true;
}

bool Navigator::canGetNextArea(const Vector& loc) {
	Vector goal;
	if (path.empty()) {
		return false;
	}
	CNavArea *topArea = TheNavMesh->GetNavAreaByID(std::get<0>(path.top()));
	if (topArea == nullptr || moveCtx->nextGoalIsLadderStart()) {
		return false;
	}
	if (lastAreaId < 0
			|| (moveCtx->nextGoalIsLadderStart() && blackboard.isOnLadder())
			|| (!moveCtx->hasGoal()
					&& moveCtx->getGoal() == topArea->GetCenter()
					&& (std::get<0>(path.top()) == lastAreaId || topArea->Contains(loc)))
							// close to next path.top()
							|| ((getPortalToTopArea(goal) && goal == moveCtx->getGoal()))) {
		return true;
	}
	CNavArea *lastArea = TheNavMesh->GetNavAreaByID(lastAreaId);
	if (lastArea == nullptr || (topArea->GetAttributes() & (NAV_MESH_CROUCH | NAV_MESH_JUMP | NAV_MESH_PRECISE | NAV_MESH_STAIRS))
					|| moveCtx->nextGoalIsLadderStart()
					// don't skip areas above and below ground height
					|| fabs(lastArea->GetCenter().z - topArea->GetCenter().z) > StepHeight
					|| fabs(loc.z - lastArea->GetCenter().z) > StepHeight) {
		return false;
	}
	auto top = path.top();
	path.pop();
	CNavArea *nextArea = path.empty() ? nullptr : TheNavMesh->GetNavAreaByID(std::get<0>(path.top()));
	bool canSkip = nextArea != nullptr && ((path.empty() && canMoveTo(finalGoal, false))
		|| (!path.empty() && getPortalToTopArea(goal) && canMoveTo(goal, false))
		// already on ladder and the next area is reached via ladder
		|| (blackboard.isOnLadder() && std::get<1>(path.top()) >= 4 && std::get<1>(path.top()) <= 5));
	path.push(top);
	if (canSkip && !blackboard.isOnLadder()) {
		// make sure we are not skipping area that will cause us to walk off of a cliff.
		nextArea->GetClosestPointOnArea(loc, &goal);
		trace_t traceResult;
		Vector dir = loc - goal;
		float totalDist = dir.Length();
		dir.NormalizeInPlace();
		float step = 8.0f;
		for (float dist = 0.0f; dist < totalDist; dist += step) {
			extern ConVar mybot_debug;
			UTIL_TraceHull(goal + dir * dist, goal + dir * (dist + step),
					Vector(0.0f, 0.0f, -JumpCrouchHeight),
					Vector(0.0f, 0.0f, 0.0f), MASK_PLAYERSOLID, CTraceFilterHitAll(),
					&traceResult, mybot_debug.GetBool());
			if (!traceResult.DidHit()) {
				return false;
			}
		}
	}
	return canSkip;
}

void Navigator::setGoalForNextArea(const Vector& loc) {
	Vector goal = finalGoal;
	lastAreaId = std::get<0>(path.top());
	dirToTop = std::get<1>(path.top());
	areaTime = 0;
	path.pop();
	if (!path.empty() && !setLadderStart()) {
		CNavArea *pathTop = TheNavMesh->GetNavAreaByID(std::get<0>(path.top()));
		if (pathTop == nullptr) {
			return;
		}
		CNavArea *lastArea = TheNavMesh->GetNavAreaByID(lastAreaId);
		if (lastArea == nullptr) {
			return;
		}
		bool crouching = (path.empty() ? lastArea : pathTop)->GetAttributes()
				& NAV_MESH_CROUCH;
		if (!getPortalToTopArea(goal)) {
			pathTop->GetClosestPointOnArea(loc, &goal);
		} else if (!canMoveTo(goal, crouching)) {
			float halfWidth;
			lastArea->ComputePortal(pathTop,
					static_cast<NavDirType>(dirToTop), &goal, &halfWidth);
			if (moveCtx->hasGoal() && !canMoveTo(goal, crouching)) {
				// if we are still moving don't set goal if we're blocked.
				return;
			}
		}
	}
	if (!moveCtx->nextGoalIsLadderStart()) {
		moveCtx->setGoal(goal);
	}
}

CNavArea* Navigator::getArea(edict_t* ent, int team) {
	CNavArea* area = TheNavMesh->GetNavArea(ent, 0);
	return area == nullptr || area->IsBlocked(team) ? TheNavMesh->GetNearestNavArea(ent) : area;
}

CNavArea* Navigator::getArea(const Vector& pos, int team) {
	CNavArea* area = TheNavMesh->GetNavArea(pos);
	if (area == nullptr || area->IsBlocked(team)) {
		// TODO: does ground need to be checked?
		area = TheNavMesh->GetNearestNavArea(pos, 10000.0f, false, true, team);
		if (area == nullptr) {
			area = TheNavMesh->GetNearestNavArea(pos, 10000.0f, false, false, team);
		}
	}
	if (area != nullptr && area->IsBlocked(team)) {
		area = nullptr;
	}
	return area;
}

bool Navigator::canMoveTo(const Vector& loc, Vector goal, float targetRadius, bool crouch) const {
	if (targetRadius > 0.0f) {
		goal = (goal - loc).Normalized() * (goal.DistTo(loc) - targetRadius) + loc;
	}
	return !moveCtx->trace(loc, goal, crouch).DidHit();
}

bool Navigator::canMoveTo(Vector goal, bool crouch) const {
	return canMoveTo(blackboard.getSelf()->getCurrentPosition(),
			goal, path.size() > 1 ? 0.0f : targetRadius, crouch);
}

bool Navigator::reachedGoal() const {
	return moveCtx->isAtTarget(finalGoal, targetRadius);
}

bool Navigator::setLadderStart() {
	if (path.empty() || std::get<1>(path.top()) < 4 || std::get<1>(path.top()) > 5) {
		return false;
	}
	CNavArea *pathTop = TheNavMesh->GetNavAreaByID(std::get<0>(path.top())),
			*lastArea = TheNavMesh->GetNavAreaByID(lastAreaId);
	if (pathTop == nullptr || lastArea == nullptr) {
		return false;
	}
	CNavLadder::LadderDirectionType dir = static_cast<CNavLadder::LadderDirectionType>(std::get<1>(path.top()) - 4);
	const NavLadderConnectVector* laddersFrom = lastArea->GetLadders(dir);
	FOR_EACH_VEC(*laddersFrom, i)
	{
		const CNavLadder* ladder = laddersFrom->Element(i).ladder;
		if (ladder->IsConnected(pathTop, dir)) {
			const Vector* start = &ladder->m_top;
			Vector end = ladder->m_bottom;
			if (dir == CNavLadder::LADDER_UP) {
				// normalize ladder height when climbing up.
				start = &ladder->m_bottom;
				// assume that the z of the next area can be stepped on.
				pathTop->GetClosestPointOnArea(*start, &end);
				end.x = ladder->m_top.x;
				end.y = ladder->m_top.y;
			}
			float delta = end.z - blackboard.getSelf()->getCurrentPosition().z;
			if (delta == 0.0f || (delta < 0.0f && delta >= -StepHeight)
					|| (delta > 0.0f && delta <= HumanHeight + MoveLadder::TARGET_OFFSET)) {
				// bot already got off the ladder
				return false;
			}
			moveCtx->setGoal(*start);
			moveCtx->setLadderEnd(end);
			moveCtx->setLadderDir(dir);
			return true;
		}
	}
	return false;
}

bool Navigator::getPortalToTopArea(Vector& portal) const {
	if (dirToTop >= NUM_DIRECTIONS || lastAreaId < 0) {
		return false;
	}
	float halfWidth;
	CNavArea *topArea = TheNavMesh->GetNavAreaByID(std::get<0>(path.top())),
			*lastArea = TheNavMesh->GetNavAreaByID(lastAreaId);
	if (topArea == nullptr || lastArea == nullptr) {
		return false;
	}
	topArea->ComputePortal(lastArea,
			OppositeDirection(static_cast<NavDirType>(dirToTop)),
			&portal, &halfWidth);
	return true;
}
