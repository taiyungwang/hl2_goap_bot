#include "DODMGDeployerProneState.h"
#include "DODMGDeployer.h"
#include "DODMGDeployerDeployState.h"
#include "DODMGDeployerStandState.h"
#include <mods/dod/util/DodPlayer.h>
#include <move/Navigator.h>
#include <move/MoveStateContext.h>
#include <player/Bot.h>
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <nav_mesh/nav.h>
#include <nav_mesh/nav_area.h>
#include <in_buttons.h>

void DODMGDeployerProneState::deploy(Blackboard& blackboard) {
	auto self = blackboard.getSelf();
	edict_t* selfEnt = self->getEdict();
	auto &buttons = blackboard.getButtons();
	if (!DodPlayer(selfEnt).isProne()) {
		if (frames++ >= 120) {
			buttons.tap(IN_ALT1);
			frames = 0;
		}
		return;
	}
	trace_t result;
	Bot::canSee(result, self->getEyesPos(), context->getViewTarget());
	if (!result.DidHit()) {
		if (buttons.tap(IN_ATTACK2)) {
			context->setState(std::make_shared<DODMGDeployerDeployState>(context));
		}
		return;
	}
	auto target = Player::getPlayer(context->getTarget());
	if (self->getVision().getVisibleEnemies().empty()
			&& (target == nullptr || !target->isInGame())) {
		context->setState(std::shared_ptr<DODMGDeployerState>(nullptr));
		return;
	}
	extern ConVar nav_slope_limit;
	if (!moveCtx) {
		moveCtx = std::make_shared<MoveStateContext>(blackboard);
	}
	const trace_t &tr = moveCtx->trace(context->getViewTarget(), false);
	if (!tr.startsolid && (tr.endpos.DistTo(result.startpos) > HalfHumanWidth * 2.0f
			|| (result.plane.normal.LengthSqr() > 0 && result.plane.normal.z <= nav_slope_limit.GetFloat()))) {
		moveCtx->setGoal(context->getViewTarget());
		CNavArea* area = Navigator::getArea(selfEnt, self->getTeam());
		moveCtx->move(area == nullptr ? NAV_MESH_INVALID: area->GetAttributes());
	} else if (buttons.tap(IN_ALT1)) {
		context->setState(std::make_shared<DODMGDeployerStandState>(context));
	}
}
