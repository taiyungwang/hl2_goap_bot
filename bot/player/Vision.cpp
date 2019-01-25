#include "Vision.h"

#include "Bot.h"
#include "Blackboard.h"
#include <util/UtilTrace.h>
#include <util/EntityUtils.h>
#include <eiface.h>
#include <ivdebugoverlay.h>

class FilterSelfAndEnemies: public CTraceFilter {
public:
	FilterSelfAndEnemies(const Blackboard& blackboard, edict_t* self,
			edict_t* target) :
			blackboard(blackboard), self(self), target(target) {
	}

	virtual ~FilterSelfAndEnemies() {
	}

	bool ShouldHitEntity(IHandleEntity *pHandleEntity, int contentsMask) {
		auto& players = blackboard.getPlayers();
		if (target != nullptr && pHandleEntity == target->GetIServerEntity()) {
			return true;
		}
		if (pHandleEntity == self->GetIServerEntity()) {
			return false;
		}
		FOR_EACH_MAP_FAST(players, i) {
			if (!players[i]->isDead()
					&& players[i]->getEdict()->GetIServerEntity()
							== pHandleEntity) {
				return false;
			}
		}
		return true;
	}

private:
	const Blackboard& blackboard;
	edict_t* self, *target;
};

bool UTIL_IsVisible(const Vector &vecAbsEnd,
		Blackboard& blackboard, edict_t* target) {
	if (target == nullptr) {
		return false;
	}
	trace_t result;
	result.fraction = 0.0f;
	auto self = blackboard.getSelf();
	Vector start = self->getEyesPos();
	CTraceFilterWorldAndPropsOnly filter;
	UTIL_TraceLine(start, vecAbsEnd, MASK_SOLID_BRUSHONLY | CONTENTS_OPAQUE,
			&filter, &result);
	return result.fraction >= 1.0
			|| (result.m_pEnt != nullptr && result.m_pEnt
			== target->GetUnknown()->GetBaseEntity());
}

void Vision::updateVisiblity(Blackboard& blackboard) {
	auto self = blackboard.getSelf();
	Vector facing = self->getFacing();
	Vector selfEyes = self->getEyesPos();
	byte pvs[MAX_MAP_CLUSTERS / 8];
	extern IVEngineServer* engine;
	if (engine->GetPVSForCluster(engine->GetClusterForOrigin(selfEyes),
			sizeof(pvs), pvs) == 0) {
		return;
	}
	float closest = INFINITY;
	const auto& players = blackboard.getPlayers();
	const Player* closestPlayer = nullptr;
	auto& visibleEnemies = blackboard.getVisibleEnemies();
	visibleEnemies.RemoveAll();
	int team = self->getTeam();
	FOR_EACH_MAP_FAST(players, i) {
		auto* target = players[i];
		if (target == self || target->isDead()
				|| (team > 0 && team == target->getTeam())) {
			continue;
		}
		edict_t* targetEnt = target->getEdict();
		Vector targetMin, targetMax;
		targetEnt->GetCollideable()->WorldSpaceSurroundingBounds(&targetMin, &targetMax);
		Vector targetPos = target->getEyesPos();
		Vector targetDir = targetPos - selfEyes;
		// check target is in FOV cone or in PVS
		float dist = targetDir.LengthSqr();
		if (dist <= 1.0f
				|| !engine->CheckBoxInPVS(targetMin, targetMax, pvs, sizeof(pvs))
				|| targetDir.Normalized().Dot(facing) <= 0.0f) {
			continue;
		}
		if (!UTIL_IsVisible(targetPos, blackboard, target->getEdict())) {
			targetPos = target->getCurrentPosition();
			targetPos.z += 31.0f; // center mass
			targetDir = targetPos - selfEyes;
			float dist = targetDir.LengthSqr();
			if (!UTIL_IsVisible(targetPos, blackboard, target->getEdict())) {
				continue;
			}
		}
		visibleEnemies.AddToTail(target);
		if (dist < closest) {
			closest = dist;
			closestPlayer = target;
			blackboard.setViewTarget(targetPos);
		}
	}
	blackboard.setTargetedPlayer(closestPlayer);
	extern ConVar mybot_debug;
	if (closestPlayer != nullptr && mybot_debug.GetBool()) {
		extern IVDebugOverlay *debugoverlay;
		debugoverlay->AddLineOverlay(selfEyes, closestPlayer->getEyesPos(), 0,
				255, 0, true,
				NDEBUG_PERSIST_TILL_NEXT_SERVER);
	}
}

