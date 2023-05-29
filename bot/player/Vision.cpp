#include "Vision.h"

#include "Bot.h"
#include "Blackboard.h"
#include <nav_mesh/nav.h>
#include <util/UtilTrace.h>
#include <util/EntityUtils.h>
#include <voice/VoiceMessage.h>
#include <eiface.h>
#include <ivdebugoverlay.h>
#include <vector>
#include <algorithm>

Vision::Vision() {
	miniMapRange = INFINITY;
}
void Vision::reset() {
	memoryDur = 0;
	targetedPlayer = 0;
}

void Vision::updateVisiblity(Bot *self) {
	Vector facing = self->getFacing().Normalized();
	Vector selfEyes = self->getEyesPos();
	byte pvs[MAX_MAP_CLUSTERS / 8];
	extern IVEngineServer *engine;
	int pvsSize = engine->GetPVSForCluster(
			engine->GetClusterForOrigin(selfEyes), sizeof(pvs), pvs);
	if (pvsSize == 0) {
		return;
	}
	nearByTeammates.clear();
	struct Visible {
		int player;
		float dist;
	};
	std::vector<Visible> enemies;
	for (auto itr : Player::getPlayers()) {
		auto *target = itr.second;
		if (target == self || !target->isInGame()) {
			continue;
		}
		Vector targetEyes = target->getEyesPos();
		if (!self->isEnemy(*target)) {
			// assume allies are visible on minimap.
			if (selfEyes.DistTo(targetEyes) < miniMapRange) {
				nearByTeammates.insert(itr.first);
			}
			continue;
		}
		Vector targetDir(targetEyes - selfEyes);
		if (targetDir.Length() <= 1.0f
				|| !engine->CheckOriginInPVS(targetEyes, pvs, pvsSize)
				|| facing.Dot(targetDir.Normalized()) <= 0.0f) {
			continue;
		}
		enemies.emplace_back();
		enemies.back().player = itr.first;
		enemies.back().dist = selfEyes.DistTo(target->getEyesPos());
	}
	std::sort(enemies.begin(), enemies.end(),
			[](const Visible &v1, const Visible &v2) {
				if (v2.dist == v2.dist) {
					return 0;
				}
				return v2.dist > v1.dist ? -1 : 1;
			});
	int lastTarget = targetedPlayer;
	targetedPlayer = 0;
	if (!visibleEnemies.empty() && enemies.empty()) {
		bool areaClear = true;
		for (auto i: visibleEnemies) {
			auto player = Player::getPlayer(i);
			if (player != nullptr && player->isInGame()) {
				areaClear = false;
				break;
			}
		}
		if (areaClear) {
			self->sendVoiceMessage(VoiceMessage::AREA_CLEAR);
		}
	}
	extern ConVar mybot_debug;
	visibleEnemies.clear();
	for (const auto &visible : enemies) {
		auto target = Player::getPlayer(visible.player);
		if (!self->canSee(*target)) {
			continue;
		}
		Vector targetPos = target->getEyesPos();
		if (targetedPlayer == 0 && self->canShoot(targetPos)) {
			self->setWantToListen(false);
			self->setViewTarget(targetPos);
			targetedPlayer = visible.player;
		} else if (mybot_debug.GetBool()) {
			extern IVDebugOverlay *debugoverlay;
			debugoverlay->AddLineOverlay(selfEyes, targetPos, 255, 255, 0, true,
			NDEBUG_PERSIST_TILL_NEXT_SERVER);
		}
		visibleEnemies.insert(visible.player);
	}
	if (targetedPlayer != 0) {
		memoryDur = 5;
	} else if (!visibleEnemies.empty() && lastTarget != 0
			&& Player::getPlayer(lastTarget) != nullptr && Player::getPlayer(lastTarget)->isInGame()) {
		if (memoryDur-- > 0) {
			self->setWantToListen(false);
			targetedPlayer = lastTarget;
		}
	}
	if (classNames.empty()) {
		return;
	}
	visibleEntities.clear();
	extern CGlobalVars *gpGlobals;
	for (int i = gpGlobals->maxClients + 1; i < gpGlobals->maxEntities; i++) {
		edict_t *ent = engine->PEntityOfEntIndex(i);
		if (ent == nullptr || ent->IsFree()
				|| classNames.find(ent->GetClassName()) == classNames.end()) {
			continue;
		}
		Vector targetPos = ent->GetCollideable()->GetCollisionOrigin();
		if (ent->GetCollideable()->GetCollisionOrigin().DistTo(selfEyes) > 300.0f
				|| facing.Dot((targetPos - selfEyes).Normalized()) <= 0.0f
				|| !self->canSee(ent)) {
			continue;
		}
		visibleEntities.insert(i);
	}
}

