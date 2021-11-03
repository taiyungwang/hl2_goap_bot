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
		const Player* player;
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
			if (self->getCurrentPosition().DistTo(target->getCurrentPosition()) < 500.0f) {
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
		visibles.back().player = target;
		visibles.back().dist = dist;
	}
	std::sort(visibles.begin(), visibles.end(),
			[](const Visible &v1, const Visible &v2) {
				if (v2.dist == v2.dist) {
					return 0;
				}
				return v2.dist > v1.dist ? -1 : 1;
			});
	const Player* lastTarget = targetedPlayer;
	targetedPlayer = nullptr;
	if (!visibleEnemies.empty() && visibles.empty()) {
		self->getVoiceMessageSender().sendMessage(std::make_shared<AreaClearVoiceMessage>(self->getEdict()));
	}
	visibleEnemies.clear();
	for(const auto &visible : visibles) {
		const Player* target = visible.player;
		if (!self->canSee(*target)) {
			continue;
		}
		Vector targetPos = target->getEyesPos();
		if (targetedPlayer == nullptr) {
			self->setWantToListen(false);
			blackboard.setViewTarget(targetPos);
			targetedPlayer = target;
		}
		extern ConVar mybot_debug;
		if (mybot_debug.GetBool()) {
			extern IVDebugOverlay *debugoverlay;
			debugoverlay->AddLineOverlay(selfEyes, targetPos, 255,
					255, 0, true,
					NDEBUG_PERSIST_TILL_NEXT_SERVER);
		}
		visibleEnemies.push_back(engine->IndexOfEdict(target->getEdict()));
	}
	if (targetedPlayer!= nullptr) {
		memoryDur = 60;
	} else if (lastTarget != nullptr && lastTarget->isInGame()) {
		memoryDur--;
		if (memoryDur > 0) {
			self->setWantToListen(false);
			targetedPlayer = lastTarget;
		}
	}
}

