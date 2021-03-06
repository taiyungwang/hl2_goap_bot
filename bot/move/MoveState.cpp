#include "MoveState.h"

#include "MoveStateContext.h"
#include "RotationManager.h"
#include "player/Blackboard.h"
#include <player/Buttons.h>
#include <player/Bot.h>
#include <util/BasePlayer.h>
#include <ivdebugoverlay.h>
#include <in_buttons.h>


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

