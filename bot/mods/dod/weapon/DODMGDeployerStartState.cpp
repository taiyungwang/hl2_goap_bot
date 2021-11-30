#include "DODMGDeployerStartState.h"
#include "DODMGDeployer.h"
#include "DODMGDeployerDeployState.h"
#include "DODMGDeployerProneState.h"
#include <player/Bot.h>
#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <nav_mesh/nav.h>
#include <util/UtilTrace.h>
#include <in_buttons.h>

class FilterSelf: public CTraceFilter {
public:
	// It does have a base, but we'll never network anything below here..

	FilterSelf(const IHandleEntity *passentity) :
			m_pPassEnt(passentity) {
	}

	virtual ~FilterSelf() {
	}

	virtual bool ShouldHitEntity(IHandleEntity *pHandleEntity,
			int contentsMask) {
		return pHandleEntity != m_pPassEnt;
	}

protected:
	const IHandleEntity *m_pPassEnt;

};

void DODMGDeployerStartState::deploy(Blackboard& blackboard) {
	Bot* self = blackboard.getSelf();
	context->setTarget(self->getVision().getTargetedPlayer());
	if (Player::getPlayer(context->getTarget()) == nullptr && blackboard.getBlocker() == nullptr) {
		self->lookStraight();
	}
	context->setViewTarget(self->getViewTarget());
	Vector pos(self->getCurrentPosition());
	pos.z += StepHeight;
	pos += (self->getViewTarget() - pos).Normalized() * HalfHumanWidth;
	trace_t result;
	extern ConVar mybot_debug;
	UTIL_TraceHull(pos, pos + (self->getViewTarget() - pos).Normalized() * HalfHumanWidth * 2.0f,
			Vector(0.0f, 0.0f, 0.0f),
			Vector(0.0f, 0.0f, HumanHeight - StepHeight),
					MASK_SOLID, FilterSelf(self->getEdict()->GetIServerEntity()),
											&result, mybot_debug.GetBool());
	auto &buttons = blackboard.getButtons();
	if (result.DidHit()) {
		if (buttons.tap(IN_ATTACK2)) {
			context->setState(std::make_shared<DODMGDeployerDeployState>(context));
			blackboard.getSelf()->setViewTarget(result.endpos);
		}
	} else if (buttons.tap(IN_ALT1)) {
		context->setState(std::make_shared<DODMGDeployerProneState>(context));
	}
}
