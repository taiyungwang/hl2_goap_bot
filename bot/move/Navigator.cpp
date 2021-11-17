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

static ConVar maxAreaTime("my_bot_max_area_time", "280");

Navigator::Navigator(Blackboard& blackboard) :
		blackboard(blackboard) {
	moveCtx = new MoveStateContext(blackboard);
}

Navigator::~Navigator() {
	delete moveCtx;
}

void Navigator::start(const Vector& goal, float targetRadius, bool sprint) {
	this->sprint = sprint;
	finalGoal = goal;
	lastAreaId = -1;
	areaTime = 0;
	this->targetRadius = targetRadius;
	moveCtx->stop();
	if (!buildPath()) {
		Msg("No path available.\n");
	}
}

CNavArea* Navigator::getLastArea() const {
	return lastAreaId < 0 ? nullptr : TheNavMesh->GetNavAreaByID(lastAreaId);
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
	CNavArea* area = getArea(self);
	if (area == nullptr) {
		return true;
	}
	CNavArea *topArea = path.empty() ? nullptr : TheNavMesh->GetNavAreaByID(path.top());
	int attributes = area->GetAttributes();
	if (canGetNextArea(loc)) {
		setGoalForNextArea(loc);
		if (!path.empty()) {
			topArea = TheNavMesh->GetNavAreaByID(path.top());
		}
	} else if (!moveCtx->hasGoal() && topArea != nullptr) {
		moveCtx->setGoal(topArea->GetCenter());
	}
	CNavArea *lastArea = TheNavMesh->GetNavAreaByID(lastAreaId);
	if (lastArea == nullptr || lastArea->IsBlocked(self->getTeam())) {
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
	if (areaTime++ > maxTime) {
		areaTime = 0;
		moveCtx->setStuck(true);
	}
	float goalDist = moveCtx->getGoal().DistTo(loc);
	if (moveCtx->hasGoal() && goalDist > 1000.0f) {
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

void Navigator::setDirToTop() {
	dirToTop = NUM_DIRECTIONS;
	if (lastAreaId < 0 || path.empty()) {
		return;
	}
	CNavArea *lastArea = TheNavMesh->GetNavAreaByID(lastAreaId),
			*topArea = TheNavMesh->GetNavAreaByID(path.top());
	if (lastArea == nullptr || topArea == nullptr) {
		return;
	}
	for (int i = 0; i < NUM_DIRECTIONS; i++) {
		if (lastArea->IsConnected(topArea, static_cast<NavDirType>(i))) {
			dirToTop = i;
			break;
		}
	}
}

class ShortestPathCostForTeam {
public:
	ShortestPathCostForTeam(int team) :
			team(team) {
	}
	float operator() ( CNavArea *area, CNavArea *fromArea, const CNavLadder *ladder, const CFuncElevator *elevator, float length ) const {
		float cost = ShortestPathCost()(area, fromArea, ladder, elevator,
				length);
		if (team > 0) {
			cost += area->GetDanger(team);
		}
		return cost;
	}
private:
	int team;
};

bool Navigator::buildPath() {
	std::stack<int> empty;
	path.swap(empty);
	const Player* self = blackboard.getSelf();
	CNavArea* goalArea = getArea(finalGoal, self->getTeam());
	if (goalArea == nullptr) {
		if (mybot_debug.GetBool()) {
			debugoverlay->AddLineOverlay(self->getEyesPos(),
					finalGoal, 255, 0, 0, true,
					NDEBUG_PERSIST_TILL_NEXT_SERVER);
		}
		Warning("Unable to find area for goal.\n");
		return false;
	}
	CNavArea* buildPathStartArea = getArea(self);
	if (buildPathStartArea == nullptr) {
		Warning("Unable to get startArea.\n");
		return false;
	}
	CNavArea* closest = nullptr;
	if (!NavAreaBuildPath(buildPathStartArea, goalArea, &finalGoal,
			ShortestPathCostForTeam(self->getTeam()), &closest, 0.0f, self->getTeam())) {
		Warning("Unable to get to goal area, %d.\n", goalArea->GetID());
		if (closest == nullptr) {
			return false;
		}
		Warning("Closest area is %d.\n",  closest->GetID());
		if (mybot_debug.GetBool() && path.size() > 0) {
			debugoverlay->AddLineOverlay(closest->GetCenter(),
					goalArea->GetCenter(), 255, 0, 0, true,
					NDEBUG_PERSIST_TILL_NEXT_SERVER);
		}
		Vector loc;
		closest->GetClosestPointOnArea(finalGoal, &loc);
		if (moveCtx->trace(loc, (finalGoal - loc).Normalized() * (finalGoal.DistTo(loc) - targetRadius) + loc,
				closest->GetAttributes() & NAV_MESH_CROUCH).DidHit()) {
			Warning("Final goal is not reachable from closest area.\n");
			return false;
		}
		goalArea = closest;
	}
	for (CNavArea* area = goalArea; area != nullptr;
			area = area->GetParent()) {
		path.push(area->GetID());
	}
	return true;
}

bool Navigator::canGetNextArea(const Vector& loc) {
	Vector goal;
	if (path.empty()) {
		return false;
	}
	CNavArea *topArea = TheNavMesh->GetNavAreaByID(path.top());
	if (topArea == nullptr) {
		return false;
	}
	if (lastAreaId < 0
			|| (!moveCtx->hasGoal()
					&& moveCtx->getGoal() == topArea->GetCenter()
					&& (path.top() == lastAreaId || topArea->Contains(loc)))
							// close to next path.top()
							|| ((getPortalToTopArea(goal) && goal == moveCtx->getGoal()))) {
		return true;
	}
	CNavArea *lastArea = TheNavMesh->GetNavAreaByID(lastAreaId);
	if (lastArea == nullptr || (topArea->GetAttributes() & (NAV_MESH_CROUCH | NAV_MESH_JUMP | NAV_MESH_PRECISE | NAV_MESH_STAIRS))
					|| moveCtx->nextGoalIsLadderStart() || blackboard.isOnLadder()
					// don't skip areas above and below ground height
					|| fabs(lastArea->GetCenter().z - topArea->GetCenter().z) > StepHeight
					|| fabs(loc.z - lastArea->GetCenter().z) > StepHeight) {
		return false;
	}
	int areaId = path.top();
	path.pop();
	CNavArea *nextArea = path.empty() ? nullptr : TheNavMesh->GetNavAreaByID(path.top());
	bool canSkip = nextArea != nullptr && ((path.empty() && canMoveTo(finalGoal, false))
		|| (!path.empty() && canMoveTo(nextArea->GetCenter(), false)));
	path.push(areaId);
	return canSkip;
}

void Navigator::setGoalForNextArea(const Vector& loc) {
	Vector goal = finalGoal;
	lastAreaId = path.top();
	areaTime = 0;
	path.pop();
	CNavArea *pathTop = nullptr;
	if (!path.empty() && !setLadderStart()) {
		CNavArea *pathTop = TheNavMesh->GetNavAreaByID(path.top());
		if (pathTop == nullptr) {
			return;
		}
		CNavArea *lastArea = TheNavMesh->GetNavAreaByID(lastAreaId);
		if (lastArea == nullptr) {
			return;
		}
		setDirToTop();
		goal = pathTop->GetCenter();
		bool crouching =
				(path.empty() ? lastArea : pathTop)->GetAttributes()
						& NAV_MESH_CROUCH;
		if (!canMoveTo(goal, crouching)
				|| fabs(lastArea->GetCenter().z - pathTop->GetCenter().z) > StepHeight) {
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

CNavArea* Navigator::getArea(const Player* player) {
	CNavArea* area = TheNavMesh->GetNavArea(player->getEdict(), 0);
	return area == nullptr ? getArea(player->getCurrentPosition(), player->getTeam()) : area;
}

bool Navigator::canMoveTo(Vector goal, bool crouch) const {
	const Vector& loc = blackboard.getSelf()->getCurrentPosition();
	if (path.size() == 1) {
		goal = (goal - loc).Normalized() * (goal.DistTo(loc) - targetRadius) + loc;
	}
	return !moveCtx->trace(goal, crouch).DidHit();
}

bool Navigator::reachedGoal() const {
	return moveCtx->isAtTarget(finalGoal, targetRadius);
}

bool Navigator::setLadderStart() {
	if (path.empty()) {
		return false;
	}
	CNavArea *pathTop = TheNavMesh->GetNavAreaByID(path.top()),
			*lastArea = TheNavMesh->GetNavAreaByID(lastAreaId);
	if (pathTop == nullptr || lastArea == nullptr) {
		return false;
	}
	for (int i = 0; i < CNavLadder::NUM_LADDER_DIRECTIONS; i++) {
		CNavLadder::LadderDirectionType dir = static_cast<CNavLadder::LadderDirectionType>(i);
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
					// the top the ladder is guaranteed to be above human height
					end.z += 3.0f * HumanHeight;
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
	}
	return false;
}

bool Navigator::getPortalToTopArea(Vector& portal) const {
	if (dirToTop >= NUM_DIRECTIONS || lastAreaId < 0) {
		return false;
	}
	float halfWidth;
	CNavArea *topArea = TheNavMesh->GetNavAreaByID(path.top()),
			*lastArea = TheNavMesh->GetNavAreaByID(lastAreaId);
	if (topArea == nullptr || lastArea == nullptr) {
		return false;
	}
	topArea->ComputePortal(lastArea,
			OppositeDirection(static_cast<NavDirType>(dirToTop)),
			&portal, &halfWidth);
	return true;
}
