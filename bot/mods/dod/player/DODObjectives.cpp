#include "DODObjectives.h"

#include "DODObjective.h"
#include <util/EntityUtils.h>
#include <mods/dod/util/DODObjectiveResource.h>
#include <nav_mesh/nav_entities.h>
#include <eiface.h>

void DODObjectives::startRound() {
	SetDefLessFunc(ctrlPointsMap);
	CUtlLinkedList<edict_t*> bombsOnMap, capArea, objRsrc;
	findEntWithMatchingName("dod_objective_resource", objRsrc);
	findEntWithMatchingName("dod_control_point", ctrlPts);
	findEntWithMatchingName("dod_bomb_target", bombsOnMap);
	findEntWithMatchingName("dod_capture_area", capArea);
	objectiveResource = new DODObjectiveResource(objRsrc[0]);
	const Vector *position = objectiveResource->getCapturePositions();
	for (int i = 0; i < objectiveResource->numCtrlPts(); i++) {
		objectives.AddToTail(new DODObjective(i, *objectiveResource));
		FOR_EACH_LL(ctrlPts, j)
		{
			ICollideable *collideable = ctrlPts[j]->GetCollideable();
			if (collideable != nullptr
					&& position[i] == collideable->GetCollisionOrigin()) {
				if (objectiveResource->getNumBombsRequired()[i] > 0) {
					detonation = true;
					addCapTarget(position[i], bombsOnMap);
				} else {
					addCapTarget(position[i], capArea);
				}
				ctrlPointsMap.Insert(ctrlPts[j], i);
			}
		}
		SearchSurroundingAreas(TheNavMesh->GetNearestNavArea(ctrlPts[i]), *objectives.Tail(), 2000.0f);
	}
	extern CGlobalVars *gpGlobals;
	extern IVEngineServer *engine;
	extern CUtlMap<int, NavEntity*> blockers;
	for (int i = gpGlobals->maxClients + 1; i < gpGlobals->maxEntities; i++) {
		edict_t *ent = engine->PEntityOfEntIndex(i);
		if (ent != nullptr && !ent->IsFree()
				&& ent->GetIServerEntity() != nullptr
				&& FClassnameIs(ent, "prop_dynamic")
				&& blockers.Find(i) == blockers.InvalidIndex()) {
			blockers.Insert(i, new CFuncNavBlocker(ent));
		}
	}
}

void DODObjectives::endRound() {
	detonation = false;
	if (objectiveResource != nullptr) {
		delete objectiveResource;
		objectiveResource = nullptr;
		ctrlPointsMap.RemoveAll();
		ctrlPts.RemoveAll();
		objectives.PurgeAndDeleteElements();
	}
}

const DODObjective* DODObjectives::getObjective(edict_t* target) const {
	auto key = ctrlPointsMap.Find(target);
	return ctrlPointsMap.IsValidIndex(key) ? objectives[ctrlPointsMap[key]] : nullptr;
}

void DODObjectives::addCapTarget(const Vector& pos,
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
				objectives.Tail()->addTarget(targets[k]);
			}
		}
	}
}
