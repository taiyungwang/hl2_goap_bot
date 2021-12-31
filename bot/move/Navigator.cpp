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

extern ConVar mybot_debug;

static ConVar maxAreaTime("my_bot_max_area_time", "80");

static const float MIN_LOOK_DIST = 50.0f;

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
	Bot* self = blackboard.getSelf();
	Vector loc = self->getCurrentPosition();
	CNavArea *topArea = path.empty() ? nullptr : TheNavMesh->GetNavAreaByID(std::get<0>(path.back()));
	if (canGetNextArea(loc)) {
		setGoalForNextArea(loc);
		if (!path.empty()) {
			topArea = TheNavMesh->GetNavAreaByID(std::get<0>(path.back()));
		}
	} else if (!moveCtx->hasGoal()) {
		CNavArea *lastArea = TheNavMesh->GetNavAreaByID(lastAreaId);
		if (lastArea == nullptr) {
			return true;
		}
		if (topArea != nullptr) {
			moveCtx->setGoal(topArea->GetCenter());
		} else {
			moveCtx->setGoal(moveCtx->getGoal() == lastArea->GetCenter()
					|| canMoveTo(finalGoal,
							lastArea->GetAttributes() & NAV_MESH_CROUCH)
							? finalGoal : lastArea->GetCenter());
		}
	}
	CNavArea *lastArea = TheNavMesh->GetNavAreaByID(lastAreaId);
	if (lastArea == nullptr) {
		return true;
	}
	int attributes = lastArea->GetAttributes();
	if ((topArea == nullptr && !path.empty()) || lastArea == nullptr
			|| lastArea->IsBlocked(self->getTeam())
			// bot fell off due to bad pathing.
			|| (!moveCtx->nextGoalIsLadderStart() && !blackboard.isOnLadder()
					&& moveCtx->getGoal().z - loc.z > JumpCrouchHeight
					&& moveCtx->getGoal().AsVector2D().DistTo(loc.AsVector2D())
							< HalfHumanWidth)) {
		return true;
	}
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
	// do look
	if (sprint || (blackboard.getBlocker() == nullptr
			&& blackboard.getSelf()->getVision().getTargetedPlayer() == 0
			&& !blackboard.isOnLadder()
			&& moveCtx->getLadderDir() == CNavLadder::NUM_LADDER_DIRECTIONS)) {
		Vector look = moveCtx->getGoal();
		look.z += HumanEyeHeight;
		if (look.DistTo(blackboard.getSelf()->getEyesPos()) > MIN_LOOK_DIST) {
			self->setViewTarget(look);
		} else {
			lookAtFurthestVisibleArea();
		}
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
	CNavArea *topArea = TheNavMesh->GetNavAreaByID(std::get<0>(path.back()));
	if (topArea == nullptr || moveCtx->nextGoalIsLadderStart()) {
		return false;
	}
	if (lastAreaId < 0
			|| (moveCtx->nextGoalIsLadderStart() && blackboard.isOnLadder())
			|| (!moveCtx->hasGoal()
					&& (moveCtx->getGoal() == topArea->GetCenter()
							// close to next path.top()
							|| (getPortalToTopArea(goal) && goal == moveCtx->getGoal())))) {
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
	int previousLast = lastAreaId;
	lastAreaId = std::get<0>(path.back());
	auto top = path.back();
	path.pop_back();
	topArea = path.empty() ? nullptr : TheNavMesh->GetNavAreaByID(std::get<0>(path.back()));
	bool canSkip = topArea != nullptr && ((path.empty() && canMoveTo(finalGoal, false))
		|| (!path.empty() && getPortalToTopArea(goal) && canMoveTo(goal, false))
		// already on ladder and the next area is reached via ladder
		|| (blackboard.isOnLadder() && std::get<1>(path.back()) >= 4 && std::get<1>(path.back()) <= 5));
	if (!blackboard.isOnLadder()) {
		// make sure we are not skipping area that will cause us to walk off of a cliff.
		Vector dir = loc - goal;
		if (path.empty()) {
			goal = finalGoal;
		}
		float totalDist = dir.Length();
		dir.NormalizeInPlace();
		static const float STEP = 8.0f;
		for (float dist = 0.0f; canSkip && dist < totalDist; dist += STEP) {
			trace_t traceResult;
			UTIL_TraceHull(goal + dir * dist, goal + dir * (dist + STEP),
					Vector(0.0f, 0.0f, -StepHeight),
					Vector(0.0f, 0.0f, 0.0f), MASK_PLAYERSOLID, CTraceFilterHitAll(),
					&traceResult, mybot_debug.GetBool());
			canSkip = traceResult.DidHit();
		}
	}
	lastAreaId = previousLast;
	path.push_back(top);
	return canSkip;
}

void Navigator::setGoalForNextArea(const Vector& loc) {
	Vector goal = finalGoal;
	lastAreaId = std::get<0>(path.back());
	areaTime = 0;
	path.pop_back();
	CNavArea *lastArea = TheNavMesh->GetNavAreaByID(lastAreaId);
	if (lastArea == nullptr) {
		return;
	}
	if (!path.empty() && !setLadderStart()) {
		CNavArea *pathTop = TheNavMesh->GetNavAreaByID(std::get<0>(path.back()));
		if (pathTop == nullptr) {
			return;
		}
		goal = pathTop->GetCenter();
		bool crouching = (path.empty() ? lastArea : pathTop)->GetAttributes()
				& NAV_MESH_CROUCH;
		if (!canMoveTo(goal, crouching) && !getPortalToTopArea(goal)) {
			pathTop->GetClosestPointOnArea(loc, &goal);
		}
		if (!canMoveTo(goal, crouching)) {
			float halfWidth;
			lastArea->ComputePortal(pathTop,
					static_cast<NavDirType>(std::get<1>(path.back())),
					&goal, &halfWidth);
			if (moveCtx->hasGoal() && !canMoveTo(goal, crouching)) {
				// if we are still moving don't set goal if we're blocked.
				return;
			}
		}
	}
	if (moveCtx->getGoal() != lastArea->GetCenter()
			&& !canMoveTo(goal, lastArea->GetAttributes() & NAV_MESH_CROUCH)) {
		goal = lastArea->GetCenter();
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
	if (path.empty() || std::get<1>(path.back()) < GO_LADDER_UP
			|| std::get<1>(path.back()) > GO_LADDER_DOWN) {
		return false;
	}
	CNavArea *pathTop = TheNavMesh->GetNavAreaByID(std::get<0>(path.back())),
			*lastArea = TheNavMesh->GetNavAreaByID(lastAreaId);
	if (pathTop == nullptr || lastArea == nullptr) {
		return false;
	}
	CNavLadder::LadderDirectionType dir = static_cast<CNavLadder::LadderDirectionType>(std::get<1>(path.back()) - 4);
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

void Navigator::lookAtFurthestVisibleArea() {
	Vector look = finalGoal;
	look.z += HumanEyeHeight;
	if (path.size() == 1) {
		blackboard.getSelf()->setViewTarget(look);
		return;
	}
	CNavArea *furthest = nullptr;
	Bot *self = blackboard.getSelf();
	for (auto itr = path.rbegin(); itr != path.rend(); itr++) {
		furthest = TheNavMesh->GetNavAreaByID(std::get<0>(*itr));
		if (furthest == nullptr) {
			return;
		}
		if (furthest->Contains(self->getCurrentPosition())) {
			continue;
		}
		look = furthest->GetCenter();
		look.z += HumanEyeHeight;
		self->setViewTarget(look);
		if (!self->canSee(look)) {
			break;
		}
	}
}


bool Navigator::getPortalToTopArea(Vector& portal) const {
	int dirToTop = std::get<1>(path.back());
	if (dirToTop >= NUM_DIRECTIONS || lastAreaId < 0) {
		return false;
	}
	float halfWidth;
	CNavArea *topArea = TheNavMesh->GetNavAreaByID(std::get<0>(path.back())),
			*lastArea = TheNavMesh->GetNavAreaByID(lastAreaId);
	if (topArea == nullptr || lastArea == nullptr) {
		return false;
	}
	topArea->ComputePortal(lastArea,
			OppositeDirection(static_cast<NavDirType>(dirToTop)),
			&portal, &halfWidth);
	return true;
}
