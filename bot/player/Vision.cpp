#include "Vision.h"

#include "Player.h"
#include "Blackboard.h"
#include <util/UtilTrace.h>
#include <util/EntityUtils.h>
#include <eiface.h>
#include <ivdebugoverlay.h>

bool FilterSelfAndEnemies::ShouldHitEntity(IHandleEntity *pHandleEntity,
		int contentsMask) {
	auto& players = blackboard.getPlayers();
	if (target != nullptr && pHandleEntity == target->GetIServerEntity()) {
		return true;
	}
	if (pHandleEntity == self->GetIServerEntity()) {
		return false;
	}
	FOR_EACH_MAP_FAST(players, i) {
		int team = players[i]->getTeam();
		if (!players[i]->isDead()
				&& players[i]->getEdict()->GetIServerEntity()
						== pHandleEntity) {
			return false;
		}
	}
	return true;
}

bool UTIL_IsVisible(const Vector &vecAbsEnd,
		Blackboard& blackboard, edict_t* target) {
	if (target == nullptr) {
		return false;
	}
	trace_t result;
	result.fraction = 0.0f;
	Vector start = blackboard.getSelf()->getEyesPos();
	for (edict_t* ignore = blackboard.getSelf()->getEdict();;
			start = result.endpos) {
		FilterSelfAndEnemies filter(blackboard, ignore, target);
		UTIL_TraceHull(start, vecAbsEnd, Vector(0.0f, -1.0f, -1.0f),
				Vector(0.0f, 1.0f, 1.0f), MASK_SHOT | MASK_VISIBLE, filter,
				&result);
		if (result.fraction >= 1.0f) {
			return true;
		}
		if (result.allsolid || result.startsolid) {
			break;
		}
		if (result.m_pEnt == target->GetUnknown()->GetBaseEntity()) {
			return true;
		}
		ignore =
				reinterpret_cast<IServerUnknown*>(result.m_pEnt)->GetNetworkable()->GetEdict();
		if (!(result.contents & CONTENTS_WINDOW) || !isBreakable(ignore)) {
			break;
		}
	}
	return false;
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
		Vector targetPos = target->getCurrentPosition(), targetDir = targetPos
				- selfEyes;
		// check target is in FOV cone or in PVS
		float dist = targetDir.LengthSqr();
		if (dist <= 1.0f
				|| !engine->CheckOriginInPVS(targetPos, pvs, sizeof(pvs))
				|| targetDir.Normalized().Dot(facing) <= 0.0f
				|| !UTIL_IsVisible(targetPos, blackboard, target->getEdict())) {
			continue;
		}
		visibleEnemies.AddToTail(target);
		if (dist < closest) {
			closest = dist;
			closestPlayer = target;
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

