#include "DODObjectives.h"

#include <util/EntityUtils.h>
#include <mods/dod/util/DODObjectiveResource.h>
#include <nav_mesh/nav_entities.h>
#include <eiface.h>

void DODObjectives::startRound() {
	SetDefLessFunc(ctrlPoints);
	CUtlLinkedList<edict_t*> points, bombsOnMap, capArea, objRsrc;
	findEntWithMatchingName("dod_objective_resource", objRsrc);
	findEntWithMatchingName("dod_control_point", points);
	findEntWithMatchingName("dod_bomb_target", bombsOnMap);
	findEntWithMatchingName("dod_capture_area", capArea);
	objectiveResource = new DODObjectiveResource(objRsrc[0]);
	const Vector *position = objectiveResource->getCapturePositions();
	for (int i = 0; i < objectiveResource->numCtrlPts(); i++) {
		capTargets.AddToTail();
		FOR_EACH_LL(points, j)
		{
			ICollideable *collideable = points[j]->GetCollideable();
			if (collideable != nullptr
					&& position[i] == collideable->GetCollisionOrigin()) {
				if (objectiveResource->getNumBombsRequired()[i] > 0) {
					detonation = true;
					addCapTarget(position[i], bombsOnMap);
				} else {
					addCapTarget(position[i], capArea);
				}
				addCapTarget(position[i], detonation ? bombsOnMap : capArea);
				ctrlPoints.Insert(points[j], i);
			}
		}
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
		ctrlPoints.RemoveAll();
		capTargets.Purge();
	}
}

int DODObjectives::getIndex(edict_t* target) const {
	auto key = ctrlPoints.Find(target);
	return ctrlPoints.IsValidIndex(key) ? ctrlPoints[key] : 1;
}

class DODBombTarget: public BaseEntity {
public:
	DODBombTarget(edict_t *ent): BaseEntity("CDODBombTarget", ent) {
	}

	int getState() {
		return get<int>("m_iState");
	}
};

bool DODObjectives::isBombInState(int idx, int state) const {
	auto& targets = capTargets[idx];
	FOR_EACH_VEC(targets, i) {
		if (DODBombTarget(targets[i]).getState() == state) {
			return true;
		}
	}
	return false;
}

bool DODObjectives::hasBombs(int idx) const {
	return objectiveResource->getNumBombsRequired()[idx] > 0;
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
				capTargets.Tail().AddToTail(targets[k]);
			}
		}
	}
}

int DODObjectives::getOwner(int idx) const {
	return objectiveResource->getOwner()[idx];
}
