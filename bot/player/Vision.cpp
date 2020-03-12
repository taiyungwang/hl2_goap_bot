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
	FilterSelfAndTarget filter(blackboard.getSelf()->getEdict()->GetIServerEntity(), target->GetIServerEntity());
	UTIL_TraceLine(start, vecAbsEnd, MASK_ALL, &filter, &result);
	return !result.DidHit();
}

void Vision::updateVisiblity(Blackboard& blackboard) {
	auto self = blackboard.getSelf();
	Vector facing = blackboard.getFacing();
	Vector selfEyes = self->getEyesPos();
	byte pvs[MAX_MAP_CLUSTERS / 8];
	extern IVEngineServer* engine;
	if (engine->GetPVSForCluster(engine->GetClusterForOrigin(selfEyes),
			sizeof(pvs), pvs) == 0) {
		return;
	}
	float closest = INFINITY;
	const auto& players = blackboard.getPlayers();
	auto& visibleEnemies = blackboard.getVisibleEnemies();
	visibleEnemies.RemoveAll();
	int team = self->getTeam();
	struct Visible {
		const Player* player;
		float dist;
	};
	CUtlVector<Visible> visibles;
	FOR_EACH_MAP_FAST(players, i) {
		auto* target = players[i];
		if (target == self || target->isDead()
				|| (team > 0 && team == target->getTeam())) {
			continue;
		}
		Vector targetMin, targetMax;
		target->getEdict()->GetCollideable()->WorldSpaceSurroundingBounds(&targetMin, &targetMax);
		Vector targetDir = target->getEyesPos() - selfEyes;
		// check target is in FOV cone or in PVS
		float dist = targetDir.LengthSqr();
		if (dist <= 1.0f
				|| !engine->CheckBoxInPVS(targetMin, targetMax, pvs, sizeof(pvs))
				|| targetDir.Normalized().Dot(facing) <= 0.0f) {
			continue;
		}
		visibles.AddToTail();
		visibles.Tail().player = target;
		visibles.Tail().dist = dist;
	}
	visibles.Sort([] (const Visible* v1, const Visible* v2) {
			if (v2->dist == v2->dist) {
				return 0;
			}
			return v2->dist > v1->dist ? -1: 1;
		});
	blackboard.setTargetedPlayer(nullptr);
	FOR_EACH_VEC(visibles, i) {
		const Player* target = visibles[i].player;
		Vector targetPos = target->getEyesPos();
		if (!UTIL_IsVisible(targetPos, blackboard, target->getEdict())) {
			targetPos = target->getCurrentPosition();
			targetPos.z += 31.0f; // center mass
			if (!UTIL_IsVisible(targetPos, blackboard, target->getEdict())) {
				continue;
			}
		}
		if (blackboard.getTargetedPlayer() == nullptr) {
			blackboard.setViewTarget(targetPos);
			blackboard.setTargetedPlayer(target);
		}
		extern ConVar mybot_debug;
		if (mybot_debug.GetBool()) {
			extern IVDebugOverlay *debugoverlay;
			debugoverlay->AddLineOverlay(selfEyes, targetPos, 0,
					255, 0, true,
					NDEBUG_PERSIST_TILL_NEXT_SERVER);
		}
		visibleEnemies.AddToTail(target->getUserId());
	}
}

