#include "MoveState.h"

#include "MoveStateContext.h"
#include "player/Blackboard.h"
#include <player/Buttons.h>
#include <player/Bot.h>
#include <ivdebugoverlay.h>
#include <in_buttons.h>

static ConVar mybot_speed_factor("mybot_speed_factor", "7.0f");

MoveState::MoveState(MoveStateContext& ctx) :
		ctx(ctx) {
	prevPos = Vector(0.0f, 0.0f, 0.0f);
	moveDur = mybot_speed_factor.GetFloat()
			* ctx.getGoal().DistTo(
					ctx.getBlackboard().getSelf()->getCurrentPosition());
	if (ctx.getType() & NAV_MESH_CROUCH) {
		moveDur *= 1.6f;
	}
}

bool MoveState::checkStuck(const Vector& currentPos) {
	float moved = currentPos.DistTo(prevPos);
	prevPos = currentPos;
	return moved < ((ctx.getType() & NAV_MESH_CROUCH) ? 0.01f : 0.1f)
			|| moveDur-- <= 0.0f;
}

void MoveState::moveStraight(const Vector& destination) const {
	Vector pos = ctx.getBlackboard().getSelf()->getCurrentPosition();
	Vector path = destination - pos;
	// get distance from current position to destination.
	int mvType = ctx.getType();
	Blackboard& blackboard = ctx.getBlackboard();
	Buttons& buttons = blackboard.getButtons();
	float speed = 320.0f; // jog speed
	if (mvType & NAV_MESH_WALK) {
		buttons.hold(IN_WALK); // walk speed
	}
	// get yaw (offset from cardinal Z) of bot.
	QAngle qAngles;
	VectorAngles(path.Normalized(), qAngles);
	if (mvType & NAV_MESH_RUN) {
		buttons.hold(IN_SPEED);
	}
	CBotCmd& cmd = blackboard.getCmd();
	SinCos(
			Blackboard::clamp180(
					blackboard.getSelf()->getAngle().y
							- Blackboard::clamp180(qAngles.y)) * M_PI / 180.0f,
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

