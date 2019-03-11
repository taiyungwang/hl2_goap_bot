#include "Jump.h"

#include "MoveStateContext.h"
#include "Stopped.h"
#include "Avoid.h"
#include "StepLeft.h"
#include <player/Bot.h>
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <navmesh/nav_entities.h>

MoveState* Jump::move(const Vector& currPos) {
	if (!ctx.getBlackboard().getButtons().jump()) {
		edict_t* blocker = ctx.getBlocker();
		Blackboard& blackboard = ctx.getBlackboard();
		blackboard.setBlocker(blocker != nullptr &&
						(Q_stristr(blocker->GetClassName(), "breakable") != nullptr
						|| Q_stristr(blocker->GetClassName(), "physics") != nullptr) ?
						blocker: nullptr);
		int team = blackboard.getSelf()->getTeam();
		if (blocker != nullptr && blackboard.getBlocker() == nullptr) {
			auto& players = blackboard.getPlayers();
			FOR_EACH_MAP_FAST(players, i) {
				if (blocker == players[i]->getEdict()
						&& (team <= 0 || team != players[i]->getTeam())) {
					blackboard.setBlocker(blocker);
					break;
				}
			}
		}
		if (blocker != nullptr
				&& Q_stristr(blocker->GetClassName(), "func_team") != nullptr) {
			extern CUtlVector<NavEntity*> blockers;
			CFuncNavBlocker* navBlocker = new CFuncNavBlocker(blocker);
			navBlocker->setBlockedTeam(blackboard.getSelf()->getTeam());
			blockers.AddToTail(navBlocker);
			return new Stopped(ctx);
		}
		if (blackboard.getBlocker() != nullptr || checkStuck(currPos)) {
			return new Stopped(ctx);
		}
		return new Avoid(ctx, new Stopped(ctx));
	}
	moveStraight(ctx.getGoal());
	return nullptr;
}
