#include "DODObjectives.h"

#include "DODObjective.h"
#include <util/EntityUtils.h>
#include <mods/dod/util/DODObjectiveResource.h>
#include <nav_mesh/nav_entities.h>
#include <eiface.h>
#include <utlmap.h>
#include <string>

DODObjectives::DODObjectives() {
	listenForGameEvent({"dod_round_active"});
}

void DODObjectives::FireGameEvent(IGameEvent *event) {
	detonation = false;
	ctrlPointsMap.clear();
	ctrlPts.RemoveAll();
	objectives.clear();
	CUtlLinkedList<edict_t*> bombsOnMap, capArea, objRsrc;
	findEntWithMatchingName("dod_objective_resource", objRsrc);
	findEntWithMatchingName("dod_control_point", ctrlPts);
	findEntWithMatchingName("dod_bomb_target", bombsOnMap);
	findEntWithMatchingName("dod_capture_area", capArea);
	DODObjectiveResource *objectiveResource = new DODObjectiveResource(objRsrc[0]);
	const Vector *position = objectiveResource->getCapturePositions();
	for (int i = 0; i < *objectiveResource->numCtrlPts(); i++) {
		objectives.push_back(
				std::make_shared<DODObjective>(i, *objectiveResource));
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
				ctrlPointsMap[ctrlPts[j]] = i;
			}
		}
		extern CNavMesh *TheNavMesh;
		SearchSurroundingAreas(TheNavMesh->GetNearestNavArea(ctrlPts[i]),
				*objectives.back(), 2000.0f);
	}
	extern CGlobalVars *gpGlobals;
	extern IVEngineServer *engine;
	extern std::unordered_map<int, CFuncNavBlocker> blockers;
	for (int i = gpGlobals->maxClients + 1; i < gpGlobals->maxEntities; i++) {
		edict_t *ent = engine->PEntityOfEntIndex(i);
		if (ent != nullptr && !ent->IsFree()
				&& ent->GetIServerEntity() != nullptr
				&& FClassnameIs(ent, "prop_dynamic")
				&& blockers.find(i) == blockers.end()) {
			blockers.emplace(i, ent);
		}
	}
}

const DODObjective* DODObjectives::getObjective(edict_t *target) const {
	auto key = ctrlPointsMap.find(target);
	return ctrlPointsMap.end() != key ? objectives[key->second].get() : nullptr;
}

void DODObjectives::addCapTarget(const Vector &pos,
		const CUtlLinkedList<edict_t*> &targets) {
	FOR_EACH_LL(targets, k)
	{
		ICollideable *collideable = targets[k]->GetCollideable();
		if (collideable != nullptr) {
			Vector targetPos = collideable->GetCollisionOrigin();
			if (targetPos.LengthSqr() == 0.0f) {
				Vector mins, maxs;
				collideable->WorldSpaceTriggerBounds(&mins, &maxs);
				targetPos = (maxs + mins) / 2.0f;
			}
			if (pos.DistTo(targetPos) < 400.0f) {
				objectives.back()->addTarget(targets[k]);
			}
		}
	}
}
