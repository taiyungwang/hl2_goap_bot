#include "CapturePointAction.h"

#include <mods/dod/util/DODObjectiveResource.h>
#include <event/EventInfo.h>
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <util/EntityUtils.h>
#include <vstdlib/random.h>
#include <in_buttons.h>

bool CapturePointAction::isDetonationMap = false;

DODObjectiveResource* CapturePointAction::objectiveResource = nullptr;

CUtlMap<edict_t*, int> CapturePointAction::ctrlPoints;

CUtlVector<CCopyableUtlVector<edict_t*>> CapturePointAction::capTarget;

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
	blackboard.lookStraight();
	blackboard.getButtons().hold(IN_DUCK);
	return false;
}

void CapturePointAction::startRound() {
	endRound();
	SetDefLessFunc(ctrlPoints);
	objectiveResource = new DODObjectiveResource();
	const Vector* position = objectiveResource->getCapturePositions();
	CUtlLinkedList<edict_t*> points, bombsOnMap, capArea;
	findEntWithMatchingName("dod_control_point", points);
	findEntWithMatchingName("dod_bomb_target", bombsOnMap);
	findEntWithMatchingName("dod_capture_area", capArea);
	for (int i = 0; i < objectiveResource->numCtrlPts(); i++) {
		capTarget.AddToTail();
		FOR_EACH_LL(points, j)
		{
			ICollideable* collideable = points[j]->GetCollideable();
			if (collideable != nullptr
					&& position[i] == collideable->GetCollisionOrigin()) {
				if (objectiveResource->getNumBombsRequired()[i] > 0) {
					isDetonationMap = true;
					addCapTarget(position[i], bombsOnMap);
				} else {
					addCapTarget(position[i], capArea);
				}
				ctrlPoints.Insert(points[j], i);
			}
		}
	}
}

void CapturePointAction::addCapTarget(const Vector& pos,
		const CUtlLinkedList<edict_t*>& targets) {
	FOR_EACH_LL(targets, k) {
		ICollideable* collideable = targets[k]->GetCollideable();
		if (collideable != nullptr) {
			Vector targetPos = collideable->GetCollisionOrigin();
			if (targetPos.LengthSqr() == 0.0f) {
				Vector mins, maxs;
				collideable->WorldSpaceTriggerBounds(&mins, &maxs);
				targetPos = (maxs + mins) / 2.0f;
			}
			if (pos.DistTo(targetPos) < 400.0f) {
			capTarget.Tail().AddToTail(targets[k]);
				}
		}
	}
}

void CapturePointAction::endRound() {
	isDetonationMap = false;
	if (objectiveResource != nullptr) {
		delete objectiveResource;
		objectiveResource = nullptr;
		ctrlPoints.RemoveAll();
		capTarget.Purge();
	}
}

bool CapturePointAction::isAvailable(edict_t* ent) {
	auto key = ctrlPoints.Find(ent);
	return ctrlPoints.IsValidIndex(key) && isAvailable(ctrlPoints[key]);
}

bool CapturePointAction::findTargetLoc() {
	if (objectiveResource == nullptr) {
		return false;
	}
	selectItem();
	if (item == nullptr) {
		return false;
	}
	setTargetLocAndRadius(capTarget[ctrlPoints[ctrlPoints.Find(item)]][0]);
	return true;
}

bool CapturePointAction::isAvailable(int idx) {
		return !isDetonationMap
				&& blackboard.getSelf()->getTeam() != objectiveResource->getOwner()[idx];
}

void CapturePointAction::selectFromActive(CUtlLinkedList<edict_t*>& active) {
	if (active.Count() == 0) {
		return;
	}
	if (active.Count() == 1) {
		item = active[0];
		return;
	}
	float totalDist = 0.0f;
	CUtlLinkedList<float> prob;
	FOR_EACH_LL(active, i) {
		prob.AddToTail(active[i]->GetCollideable()->GetCollisionOrigin().DistTo(blackboard.getSelf()->getCurrentPosition()));
		totalDist += prob[prob.Tail()];
	}
	FOR_EACH_LL(prob, i) {
		prob[i] = (totalDist - prob[i]) / totalDist / 2.0f;
		if (i > 0) {
			prob[i] += prob[i - 1];
		}
	}
	float choice = RandomFloat(0, 1.0f);
	FOR_EACH_LL(prob, i) {
		if (choice < prob[i]) {
			item = active[i];
			break;
		}
	}
}
