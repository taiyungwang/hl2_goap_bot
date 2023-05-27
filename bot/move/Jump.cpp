#include "Jump.h"

#include "MoveStateContext.h"
#include "StepLeft.h"
#include "Avoid.h"
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <player/Buttons.h>
#include <util/UtilTrace.h>

class IgnoreSelfAndTeammates: public CTraceFilter {
public:
	IgnoreSelfAndTeammates(edict_t* self): self(self) {
	}

	virtual ~IgnoreSelfAndTeammates() {
	}

	bool ShouldHitEntity(IHandleEntity *pHandleEntity,
			int contentsMask) override {
		if (pHandleEntity == self->GetIServerEntity()) {
			return false;
		}
		int team = Player::getPlayer(self)->getTeam();
		extern IVEngineServer* engine;
		int targetIdx = engine->IndexOfEdict(reinterpret_cast<IServerUnknown*>(const_cast<IHandleEntity*>(pHandleEntity))
						->GetNetworkable()->GetEdict());
		extern CGlobalVars *gpGlobals;
		return team == 0 || targetIdx == 0 || gpGlobals->maxClients < targetIdx
				|| Player::getPlayer(targetIdx)->getTeam() != team;
	}
private:
	edict_t* self;
};

MoveState* Jump::move(const Vector& currPos) {
	Vector pos(currPos), goal((ctx.getGoal() - currPos).Normalized() * 16.0f + pos);
	pos.z += 48.0f;
	goal.z = pos.z;
	if ((!ctx.getBlackboard().getSelf()->isOnLadder() && ctx.trace(pos, goal, true,
			IgnoreSelfAndTeammates(ctx.getBlackboard().getSelf()->getEdict())).DidHit())
			|| !ctx.getBlackboard().getButtons().jump()) {
		return new Avoid(ctx, new StepLeft(ctx));
	}
	moveStraight(ctx.getGoal());
	return nullptr;
}
