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

static ConVar mybotStuckThreshold("mybot_stuck_threshold", "90.0f");

Navigator::Navigator(Blackboard& blackboard) :
		blackboard(blackboard) {
	moveCtx = new MoveStateContext(blackboard);
}

Navigator::~Navigator() {
	delete moveCtx;
}

void Navigator::start(const Vector& goal, float targetRadius) {
	finalGoal = goal;
	lastArea = nullptr;
	areaTime = 0;
	this->targetRadius = targetRadius;
	moveCtx->stop();
	if (!buildPath()) {
		Msg("No path available.\n");
	} else {
		moveCtx->setGoal(path.top()->GetCenter());
	}
}

bool Navigator::step() {
	if (moveCtx->isStuck() || reachedGoal() || blackboard.getBlocker() != nullptr
			|| (lastArea == nullptr && path.empty())) {
		return true;
	}
	if (!checkCanMove()) {
		return false;
	}
	const Player* self = blackboard.getSelf();
	Vector loc = self->getCurrentPosition();
	CNavArea* area = getCurrentArea();
	if (area == nullptr) {
		return true;
	}
	int attributes = path.empty() ? lastArea->GetAttributes() : path.top()->GetAttributes();
	if (canGetNextArea(loc)) {
		setGoalForNextArea(loc);
	} else if (!moveCtx->hasGoal() && !path.empty()) {
		moveCtx->setGoal(path.top()->GetCenter());
	}
	if (lastArea->IsBlocked(self->getTeam())) {
		return true;
	}
	if (mybot_debug.GetBool() && !path.empty()) {
		path.top()->Draw();
	}
	// magic number from https://developer.valvesoftware.com/wiki/Dimensions#Horizontal_.28To_Equal_Height.29
	if (((attributes & NAV_MESH_JUMP) ||
			((attributes & NAV_MESH_CROUCH) && !(area->GetAttributes() & NAV_MESH_CROUCH)))
			&& moveCtx->getGoal().AsVector2D().DistTo(loc.AsVector2D()) > 136.0f) {
		attributes = NAV_MESH_INVALID;
	}
	int maxTime = maxAreaTime.GetInt();
	if ((attributes & NAV_MESH_CROUCH) || moveCtx->nextGoalIsLadderStart()) {
		maxTime *= 1.5f;
	}
	if (areaTime++ > maxTime
			&& BasePlayer(self->getEdict()).getVelocity().Length() < mybotStuckThreshold.GetFloat()) {
		areaTime = 0;
		moveCtx->setStuck(true);
	}
	float goalDist = moveCtx->getGoal().DistTo(loc);
	if (moveCtx->hasGoal() && goalDist > 1000.0f) {
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
	if (lastArea == nullptr || path.empty()) {
		return;
	}
	for (int i = 0; i < NUM_DIRECTIONS; i++) {
		if (lastArea->IsConnected(path.top(), static_cast<NavDirType>(i))) {
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
	std::stack<CNavArea*> empty;
	path.swap(empty);
	const Player* self = blackboard.getSelf();
	CNavArea* goalArea = getGoalArea(finalGoal);
	if (goalArea == nullptr) {
		if (mybot_debug.GetBool()) {
			debugoverlay->AddLineOverlay(self->getEyesPos(),
					finalGoal, 255, 0, 0, true,
					NDEBUG_PERSIST_TILL_NEXT_SERVER);
		}
		Warning("Unable to find area for goal.\n");
		return false;
	}
	CNavArea* buildPathStartArea = getCurrentArea();
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
		float dist = closest->GetCenter().DistTo(finalGoal);
		if (dist > 300.0f) {
			Warning("Closest area is too far away: %f.\n", dist);
			return false;
		}
		goalArea = closest;
	}
	for (CNavArea* area = goalArea; area != nullptr;
			area = area->GetParent()) {
		path.push(area);
	}
	return true;
}

bool Navigator::canGetNextArea(const Vector& loc) const {
	Vector goal;
	return !path.empty()
			&& ((!moveCtx->hasGoal()
					&& (path.top() == lastArea || path.top()->Contains(loc)
							// close to next path.top()
							|| (getPortalToTopArea(goal) && goal == moveCtx->getGoal())
							|| moveCtx->getGoal() == path.top()->GetCenter()))
					|| (canMoveTo(moveCtx->getGoal(), lastArea->GetAttributes() & NAV_MESH_CROUCH)
							&& !(lastArea->GetAttributes() & (NAV_MESH_CROUCH | NAV_MESH_JUMP | NAV_MESH_PRECISE))
							&& (!moveCtx->nextGoalIsLadderStart() && !blackboard.isOnLadder())
							// don't skip areas above and below ground height
							&& fabs(lastArea->GetCenter().z - path.top()->GetCenter().z) <= StepHeight
							&& fabs(loc.z - lastArea->GetCenter().z) <= StepHeight
							&& (getPortalToTopArea(goal)
									&& canMoveTo(goal, lastArea->GetAttributes() & NAV_MESH_CROUCH))));
}

void Navigator::setGoalForNextArea(const Vector& loc) {
	Vector goal = finalGoal;
	lastArea = path.top();
	areaTime = 0;
	path.pop();
	if (!path.empty() && !setLadderStart()) {
		setDirToTop();
		goal = path.top()->GetCenter();
		bool crouching =
				(path.empty() ? lastArea : path.top())->GetAttributes()
						& NAV_MESH_CROUCH;
		if (!canMoveTo(goal, crouching)
				|| fabs(lastArea->GetCenter().z - path.top()->GetCenter().z) > StepHeight) {
			if (!getPortalToTopArea(goal)) {
				path.top()->GetClosestPointOnArea(loc, &goal);
			} else if (!canMoveTo(goal, crouching)) {
				float halfWidth;
				lastArea->ComputePortal(path.top(),
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

CNavArea* Navigator::getCurrentArea() const {
	const auto self = blackboard.getSelf();
	CNavArea* area = TheNavMesh->GetNavArea(self->getEdict(), 0);
	if (area == nullptr)
		area = getGoalArea(self->getCurrentPosition());
	return area;
}

CNavArea* Navigator::getGoalArea(const Vector& pos) const {
	int team = blackboard.getSelf()->getTeam();
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

bool Navigator::canMoveTo(Vector goal, bool crouch) const {
	const Vector& loc = blackboard.getSelf()->getCurrentPosition();
	if (path.size() == 1) {
		goal = (goal - loc).Normalized() * (goal.DistTo(loc) - targetRadius) + loc;
	}
	return !moveCtx->trace(goal, crouch).DidHit();
}

bool Navigator::reachedGoal() const {
	return blackboard.getSelf()->getCurrentPosition().DistTo(finalGoal) < targetRadius
		|| (path.empty() && moveCtx->getGoal() == finalGoal
			&& moveCtx->reachedGoal(targetRadius));
}

bool Navigator::setLadderStart() {
	if (path.empty()) {
		return false;
	}
	for (int i = 0; i < CNavLadder::NUM_LADDER_DIRECTIONS; i++) {
		CNavLadder::LadderDirectionType dir = static_cast<CNavLadder::LadderDirectionType>(i);
		const NavLadderConnectVector* laddersFrom = lastArea->GetLadders(dir);
		FOR_EACH_VEC(*laddersFrom, i)
		{
			const CNavLadder* ladder = laddersFrom->Element(i).ladder;
			if (ladder->IsConnected(path.top(), dir)) {
				const Vector* start = &ladder->m_top;
				Vector end = ladder->m_bottom;
				if (dir == CNavLadder::LADDER_UP) {
					// normalize ladder height when climbing up.
					start = &ladder->m_bottom;
					// assume that the z of the next area can be stepped on.
					path.top()->GetClosestPointOnArea(*start, &end);
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
	if (dirToTop >= NUM_DIRECTIONS || lastArea == nullptr) {
		return false;
	}
	float halfWidth;
	path.top()->ComputePortal(lastArea, OppositeDirection(static_cast<NavDirType>(dirToTop)),
			&portal, &halfWidth);
	return true;
}
