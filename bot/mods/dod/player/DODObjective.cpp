#include "DODObjective.h"

#include <mods/dod/util/DODObjectiveResource.h>
#include <util/BaseEntity.h>
#include <util/UtilTrace.h>
#include <util/EntityUtils.h>
#include <eiface.h>
#include <utlvector.h>

bool DODObjective::operator()(CNavArea *area, CNavArea *priorArea,
		float travelDistanceSoFar) {
	const auto& areaHideSpots = *area->GetHidingSpots();
	FOR_EACH_VEC(areaHideSpots, i) {
		trace_t result;
		Vector hideSpot = areaHideSpots[i]->GetPosition();
		hideSpot.z += HumanCrouchEyeHeight;
		UTIL_TraceLine(rsc.getCapturePositions()[idx],
				hideSpot, MASK_NPCWORLDSTATIC, nullptr, &result);
		extern IServerGameEnts *servergameents;
		edict_t* hit = result.m_pEnt == nullptr ? nullptr: servergameents->BaseEntityToEdict(result.m_pEnt);
		if (hit == nullptr || !FClassnameIs(hit, "worldspawn")) {
			hideSpots.AddToTail(areaHideSpots[i]->GetID());
		}
	}
	return true;
}

class DODBombTarget: public BaseEntity {
public:
	DODBombTarget(edict_t *ent) :
			BaseEntity("CDODBombTarget", ent) {
	}

	int getState() {
		return get<int>("m_iState");
	}
};

bool DODObjective::hasBombTargetInState(BombState state) const {
	FOR_EACH_VEC(targets, i)
	{
		if (DODBombTarget(targets[i]).getState() == static_cast<int>(state)) {
			return true;
		}
	}
	return false;
}

int DODObjective::getOwner() const {
	return rsc.getOwner()[idx];
}

bool DODObjective::hasBombs() const {
	return rsc.getNumBombsRequired()[idx] > 0;
}
