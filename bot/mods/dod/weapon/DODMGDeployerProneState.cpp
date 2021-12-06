#include "DODMGDeployerProneState.h"
#include "DODMGDeployer.h"
#include <mods/dod/util/DodPlayer.h>
#include <move/Navigator.h>
#include <move/MoveStateContext.h>
#include <player/Bot.h>
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <weapon/Weapon.h>
#include <nav_mesh/nav.h>
#include <nav_mesh/nav_area.h>
#include <in_buttons.h>

const int DODMGDeployerProneState::PRONE_TIMEOUT = 120,
		DODMGDeployerProneState::DEPLOY_TIMEOUT = PRONE_TIMEOUT + 5;

void DODMGDeployerProneState::deploy(Blackboard& blackboard) {
	auto self = blackboard.getSelf();
	edict_t* selfEnt = self->getEdict();
	if (!DodPlayer(selfEnt).isProne()) {
		if (wait++ == 0) {
			targetIdx = self->getVision().getTargetedPlayer();
			blackboard.getButtons().hold(IN_ALT1);
		} else if (wait++ >= PRONE_TIMEOUT) {
			context->setState(std::shared_ptr<DODMGDeployerState>(nullptr));
		}
		return;
	}
	auto target = Player::getPlayer(targetIdx);
	if (target == nullptr
		// if there are no enemies just deploy to end the deploy cycle.
			|| (self->getVision().getVisibleEnemies().empty()
					&& !target->isInGame())) {
		if (wait++ >= DEPLOY_TIMEOUT) {
			context->setState(std::shared_ptr<DODMGDeployerState>(nullptr));
		} else if (!context->getWeapon().isDeployed()) {
			blackboard.getButtons().tap(IN_ATTACK2);
		}
		return;
	}
	trace_t result;
	Bot::canSee(result, self->getEyesPos(), self->getViewTarget());
	if (!moveCtx) {
		moveCtx = std::make_shared<MoveStateContext>(blackboard);
	}
	moveCtx->setGoal(self->getViewTarget());
	CNavArea* area = Navigator::getArea(selfEnt, self->getTeam());
	moveCtx->move(area == nullptr ? NAV_MESH_INVALID: area->GetAttributes());
	const auto& tr = moveCtx->getTraceResult();
	if (wait++ >= DEPLOY_TIMEOUT && (tr.startsolid
			|| (tr.endpos.DistTo(tr.startpos) < HalfHumanWidth))) {
		context->setState(std::shared_ptr<DODMGDeployerState>(nullptr));
	} else if (self->getVision().getTargetedPlayer() > 0 || !result.DidHit()) {
		if (context->getWeapon().isDeployed() && wait++ >= DEPLOY_TIMEOUT) {
			context->setState(std::shared_ptr<DODMGDeployerState>(nullptr));
		} else {
			blackboard.getButtons().tap(IN_ATTACK2);
		}
	}
}
