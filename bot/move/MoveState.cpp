#include "MoveState.h"

#include "MoveStateContext.h"
#include "RotationManager.h"
#include "player/Blackboard.h"
#include <player/Buttons.h>
#include <player/Bot.h>
#include <util/BasePlayer.h>
#include <ivdebugoverlay.h>
#include <in_buttons.h>

static ConVar mybot_speed_factor("mybot_speed_factor", "3.0f");
static ConVar mybot_stuck_threshold("mybot_stuck_threshold", "0.02f");

MoveState::MoveState(MoveStateContext& ctx) :
		ctx(ctx) {
	prevPos = Vector(0.0f, 0.0f, 0.0f);
	durLimit = mybot_speed_factor.GetFloat()
			* ctx.getGoal().DistTo(
					ctx.getBlackboard().getSelf()->getCurrentPosition());
	if (ctx.getType() & NAV_MESH_CROUCH) {
		durLimit *= 1.6f;
	}
}

bool MoveState::checkStuck(const Vector& currentPos, const Vector& goal) {
	float moved = prevPos.AsVector2D().DistTo(goal.AsVector2D())
			- currentPos.AsVector2D().DistTo(goal.AsVector2D());
	prevPos = currentPos;
	return moveDur++ > durLimit ||
			(moveDur > 100 && moved < mybot_stuck_threshold.GetFloat());
}

void MoveState::moveStraight(const Vector& destination) const {
	// get distance from current position to destination.
	int mvType = ctx.getType();
	Blackboard& blackboard = ctx.getBlackboard();
	Vector pos = blackboard.getSelf()->getCurrentPosition();
	Vector path = destination - pos;
	Buttons& buttons = blackboard.getButtons();
	float speed = 450.0f; // jog speed
	if (mvType & NAV_MESH_WALK) {
		buttons.hold(IN_WALK); // walk speed
	}
	// get yaw (offset from cardinal Z) of bot.
	QAngle pathAngle;
	VectorAngles(path.Normalized(), pathAngle);
	if (mvType & NAV_MESH_RUN) {
		buttons.hold(IN_SPEED);
	}
	CBotCmd& cmd = blackboard.getCmd();
	SinCos(RotationManager::clamp180(blackboard.getSelf()->getAbsoluteAngle().y
			- RotationManager::clamp180(pathAngle.y)) * M_PI / 180.0f,
			&cmd.sidemove, &cmd.forwardmove);
	cmd.forwardmove *= speed;
	cmd.sidemove *= speed;
	if (mvType & NAV_MESH_JUMP) {
		buttons.jump();
	} else if (mvType & NAV_MESH_CROUCH) {
		buttons.hold(IN_DUCK);
	}
	extern ConVar mybot_debug;
	if (mybot_debug.GetBool()) {
		extern IVDebugOverlay *debugoverlay;
		debugoverlay->AddLineOverlay(pos, destination, 255, 255, 0, true,
		NDEBUG_PERSIST_TILL_NEXT_SERVER);
	}
}

