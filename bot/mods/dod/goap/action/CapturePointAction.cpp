#include "CapturePointAction.h"

#include <mods/dod/util/DODObjectiveResource.h>
#include <event/EventInfo.h>
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <player/Player.h>
#include <util/EntityUtils.h>
#include <vstdlib/random.h>
#include <in_buttons.h>

DODObjectiveResource* CapturePointAction::objectiveResource = nullptr;
CUtlMap<edict_t*, int> CapturePointAction::capPoints;

CapturePointAction::CapturePointAction(Blackboard& blackboard) :
		GoToEntityAction(blackboard, "dod_control_point") {
	allItemsVisible = true;
	precond.Insert(WorldProp::ROUND_STARTED, true);
	effects = {WorldProp::ALL_POINTS_CAPTURED, true};
}

bool CapturePointAction::execute() {
	bool done = GoToEntityAction::execute();
	if (!done) {
		blackboard.getButtons().hold(IN_DUCK);
	}
	return done;
}

void CapturePointAction::startRound() {
	endRound();
	SetDefLessFunc(capPoints);
	objectiveResource = new DODObjectiveResource();
	const Vector* position = objectiveResource->getCapturePositions();
	CUtlLinkedList<edict_t*> points;
	findEntWithSubStrInName("dod_control_point", points);
	for (int i = 0; i < objectiveResource->numCtrlPts(); i++) {
		FOR_EACH_LL(points, j)
		{
			ICollideable* collideable = points[j]->GetCollideable();
			if (collideable != nullptr
					&& position[i] == collideable->GetCollisionOrigin()) {
				capPoints.Insert(points[j], i);
			}
		}
	}
}

void CapturePointAction::endRound() {
	if (objectiveResource != nullptr) {
		delete objectiveResource;
		objectiveResource = nullptr;
		capPoints.RemoveAll();
	}
}

bool CapturePointAction::isAvailable(edict_t* ent) const {
	auto key = capPoints.Find(ent);
	return capPoints.IsValidIndex(key)
			&& blackboard.getSelf()->getTeam()
					!= objectiveResource->getOwner()[capPoints[key]];
}

void CapturePointAction::selectItem(CUtlLinkedList<edict_t*>& active) {
	if (active.Count() == 0) {
		return;
	}
	if (RandomInt(0, 1) == 0) {
		GoToEntityAction::selectItem(active);
		return;
	}
	item = active[RandomInt(0, active.Count() - 1)];
}
