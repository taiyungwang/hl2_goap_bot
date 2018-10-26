#include "CapturePointAction.h"

#include <mods/dod/util/DODObjectiveResource.h>
#include <event/EventInfo.h>
#include <player/Blackboard.h>
#include <player/Player.h>
#include <util/EntityUtils.h>
#include <vstdlib/random.h>
#include <in_buttons.h>

bool CapturePointAction::isDetonationMap = false;

DODObjectiveResource* CapturePointAction::objectiveResource = nullptr;

CUtlMap<edict_t*, int> CapturePointAction::capPoints;

CUtlVector<CCopyableUtlVector<edict_t*>> CapturePointAction::bombs;

CapturePointAction::CapturePointAction(Blackboard& blackboard) :
		GoToConsumableEntityAction(blackboard, "dod_control_point") {
	allItemsVisible = true;
	precond.Insert(WorldProp::ROUND_STARTED, true);
	effects = {WorldProp::ALL_POINTS_CAPTURED, true};
}

bool CapturePointAction::execute() {
	if (!GoToConsumableEntityAction::execute()) {
		return false;
	}
	if (!GoToAction::postCondCheck() || isDepleted()) {
		return true;
	}
	blackboard.getButtons().hold(IN_DUCK);
	return false;
}

void CapturePointAction::startRound() {
	endRound();
	SetDefLessFunc(capPoints);
	objectiveResource = new DODObjectiveResource();
	const Vector* position = objectiveResource->getCapturePositions();
	CUtlLinkedList<edict_t*> points, bombsOnMap;
	findEntWithSubStrInName("dod_control_point", points);
	findEntWithSubStrInName("dod_bomb_target", bombsOnMap);
	for (int i = 0; i < objectiveResource->numCtrlPts(); i++) {
		bombs.AddToTail();
		FOR_EACH_LL(points, j)
		{
			ICollideable* collideable = points[j]->GetCollideable();
			if (collideable != nullptr
					&& position[i] == collideable->GetCollisionOrigin()) {
				if (objectiveResource->getNumBombsRequired()[i] > 0) {
					isDetonationMap = true;
					FOR_EACH_LL(bombsOnMap, k) {
						ICollideable* collideable = bombsOnMap[k]->GetCollideable();
						if (collideable != nullptr
								&& position[i].DistTo(collideable->GetCollisionOrigin())
								< 400.0f) {
							bombs.Tail().AddToTail(bombsOnMap[k]);
						}
					}
				}
				capPoints.Insert(points[j], i);
			}
		}
	}
}

void CapturePointAction::endRound() {
	isDetonationMap = false;
	if (objectiveResource != nullptr) {
		delete objectiveResource;
		objectiveResource = nullptr;
		capPoints.RemoveAll();
		bombs.Purge();
	}
}

bool CapturePointAction::isAvailable(edict_t* ent) const {
	auto key = capPoints.Find(ent);
	return capPoints.IsValidIndex(key) && isAvailable(capPoints[key]);
}

bool CapturePointAction::isAvailable(int idx) const {
		return !isDetonationMap
				&& blackboard.getSelf()->getTeam() != objectiveResource->getOwner()[idx];
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
