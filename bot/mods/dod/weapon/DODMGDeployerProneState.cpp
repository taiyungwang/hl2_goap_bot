#include "DODMGDeployerProneState.h"
#include "DODMGDeployer.h"
#include "DODMGDeployerStandState.h"
#include <mods/dod/util/DodPlayer.h>
#include <move/Navigator.h>
#include <move/MoveStateContext.h>
#include <player/Bot.h>
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <weapon/Weapon.h>
#include <in_buttons.h>

void DODMGDeployerProneState::deploy(Blackboard& blackboard) {
	auto self = blackboard.getSelf();
	edict_t* selfEnt = self->getEdict();
	if (!DodPlayer(selfEnt).isProne()) {
		if (wait++ == 0) {
			targetIdx = self->getVision().getTargetedPlayer();
			blackboard.getButtons().hold(IN_ALT1);
		} else if (wait >= PRONE_TIMEOUT) {
			context->setState(std::shared_ptr<DODMGDeployerState>(nullptr));
		}
		return;
	}
	auto target = Player::getPlayer(targetIdx);
	trace_t result;
	if (target != nullptr) {
		Bot::canSee(result, self->getEyesPos(), self->getViewTarget());
	}
	bool seeEnemies = !self->getVision().getVisibleEnemies().empty()
			|| (target != nullptr && !result.DidHit());
	if (context->getWeapon().isDeployed()
			|| (target == nullptr && wait++ >= PRONE_TIMEOUT)) {
		context->setState(std::shared_ptr<DODMGDeployerState>(nullptr));
		return;
	}
	if (seeEnemies || target == nullptr) {
		blackboard.getButtons().tap(IN_ATTACK2);
		return;
	}
	if (!moveCtx) {
		moveCtx = std::make_shared<MoveStateContext>(blackboard);
		moveCtx->setGoal(self->getViewTarget());
		moveCtx->traceMove(false);
	}
	const auto& tr = moveCtx->getTraceResult();
	if ((tr.startsolid || tr.endpos.DistTo(tr.startpos) < HalfHumanWidth)
			&& (result.startsolid || result.endpos.DistTo(result.startpos) < 2.0f * HalfHumanWidth)) {
		context->setState(std::make_shared<DODMGDeployerStandState>(context));
	} else {
		moveCtx->move(NAV_MESH_INVALID);
	}
}
