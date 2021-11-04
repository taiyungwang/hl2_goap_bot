#include "Vision.h"

#include "Bot.h"
#include "Blackboard.h"
#include <voice/AreaClearVoiceMessage.h>
#include <voice/VoiceMessageSender.h>
#include <util/UtilTrace.h>
#include <util/EntityUtils.h>
#include <eiface.h>
#include <ivdebugoverlay.h>
#include <vector>
#include <algorithm>

Vision::Vision() {
	miniMapRange = INFINITY;
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
	nearByTeammates.clear();
	struct Visible {
		int player;
		float dist;
	};
	std::vector<Visible> visibles;
	for (auto itr: Player::getPlayers()) {
		auto* target = itr.second;
		if (target == self || !target->isInGame()) {
			continue;
		}
		if (!self->isEnemy(*target)) {
			// assume allies are visible on minimap.
			if (self->getCurrentPosition().DistTo(target->getCurrentPosition()) < miniMapRange) {
				nearByTeammates.push_back(itr.first);
			}
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
		visibles.emplace_back();
		visibles.back().player = itr.first;
		visibles.back().dist = dist;
	}
	std::sort(visibles.begin(), visibles.end(),
			[](const Visible &v1, const Visible &v2) {
				if (v2.dist == v2.dist) {
					return 0;
				}
				return v2.dist > v1.dist ? -1 : 1;
			});
	int lastTarget = targetedPlayer;
	targetedPlayer = 0;
	if (!visibleEnemies.empty() && visibles.empty()) {
		self->getVoiceMessageSender().sendMessage(std::make_shared<AreaClearVoiceMessage>(self->getEdict()));
	}
	visibleEnemies.clear();
	for(const auto &visible : visibles) {
		auto target = Player::getPlayer(visible.player);
		if (!self->canSee(*target)) {
			continue;
		}
		Vector targetPos = target->getEyesPos();
		if (targetedPlayer == 0) {
			self->setWantToListen(false);
			blackboard.setViewTarget(targetPos);
			targetedPlayer = visible.player;
		}
		extern ConVar mybot_debug;
		if (mybot_debug.GetBool()) {
			extern IVDebugOverlay *debugoverlay;
			debugoverlay->AddLineOverlay(selfEyes, targetPos, 255,
					255, 0, true,
					NDEBUG_PERSIST_TILL_NEXT_SERVER);
		}
		visibleEnemies.push_back(visible.player);
	}
	if (targetedPlayer != 0) {
		memoryDur = 60;
	} else if (lastTarget != 0 && Player::getPlayer(lastTarget)->isInGame()) {
		memoryDur--;
		if (memoryDur > 0) {
			self->setWantToListen(false);
			targetedPlayer = lastTarget;
		}
	}
}

