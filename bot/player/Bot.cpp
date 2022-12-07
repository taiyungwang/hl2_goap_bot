#include "Bot.h"

#include "Blackboard.h"
#include "Vision.h"
#include "World.h"
#include "FilterSelfAndEnemies.h"
#include <event/EventInfo.h>
#include <goap/action/GoToAction.h>
#include <goap/GoalManager.h>
#include <move/Navigator.h>
#include <move/RotationManager.h>
#include <voice/AreaClearVoiceMessage.h>
#include <voice/VoiceMessageSender.h>
#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>
#include <nav_mesh/nav_area.h>
#include <util/SimpleException.h>
#include <util/BasePlayer.h>
#include <util/EntityUtils.h>
#include <util/UtilTrace.h>
#include <ivdebugoverlay.h>
#include <IEngineTrace.h>
#include <in_buttons.h>

PlayerClasses Bot::CLASSES = nullptr;

static ConVar mybot_rot_speed("mybot_rot_speed", "0.3", 0,
		"determines rotational acceleration rate in degrees");

static ConVar mybot_mimic("mybot_mimic", "0");

static ConVar mybotAimVar("mybot_aim_variance", "1.0f", 0,
		"range of randomness for a bot's aim");

static ConVar mybotDangerAmt("mybot_danger_amount", "3.0f", 0,
		"Amount of 'danger' to add to an area when a bot is killed.");

Bot::~Bot() {
	delete blackboard;
	delete world;
	delete planner;
	if (playerClassVar != nullptr) {
		delete playerClassVar;
	}
}

void Bot::think() {
	try {
		Player::think();
		CBotCmd& cmd = blackboard->getCmd();
		if (!inGame) {
			blackboard->getButtons().tap(IN_ATTACK);
		} else if (getPlayerClass() != desiredClassId) {
			extern IServerPluginHelpers* helpers;
			helpers->ClientCommand(getEdict(), (*CLASSES)[getTeam() - 2][desiredClassId]);
		} else {
			vision.updateVisiblity(this);
			wantToListen = true;
			cmd.Reset();
			if ((world->think(*blackboard) || resetPlanner)
					&& !blackboard->isOnLadder()) {
				resetPlanner = false;
				planner->resetPlanning(false);
			}
			planner->execute();
			listen();
			extern ConVar mybot_debug;
			if (mybot_debug.GetBool()) {
				extern IVDebugOverlay *debugoverlay;
				debugoverlay->AddLineOverlay(getEyesPos(), viewTarget, 0,
						255, vision.getTargetedPlayer() > 0 ? 255 : 0, true,
						NDEBUG_PERSIST_TILL_NEXT_SERVER);
			}
			VectorAngles(viewTarget - getEyesPos(), cmd.viewangles);
			if (aiming && vision.getTargetedPlayer() > 0) {
				cmd.viewangles.x += RandomFloat(-mybotAimVar.GetFloat(), mybotAimVar.GetFloat());
				cmd.viewangles.y += RandomFloat(-mybotAimVar.GetFloat(), mybotAimVar.GetFloat());
			}
			rotation.getUpdatedPosition(cmd.viewangles, getFacingAngle(),
					mybot_rot_speed.GetFloat());
		}
		cmd.buttons = blackboard->getButtons().getPressed();
		extern CGlobalVars *gpGlobals;
		cmd.tick_count = gpGlobals->tickcount;
		if (mybot_mimic.GetBool()) {
			cmd = Player::getPlayers()[1]->getInfo()->GetLastUserCommand();
		}
		extern IBotManager *botmanager;
		if (!hookEnabled) {
			botmanager->GetBotController(getEdict())->RunPlayerMove(&cmd);
		}
	} catch (const Exception& e) {
		Error("%s: %s", getName(), e.what());
	}
}

bool Bot::handle(EventInfo* event) {
	Player::handle(event);
	std::string name(event->getName());
	int eventUserId = event->getInt("userid");
	// bot owns this event.
	if (eventUserId != getUserId()) {
		return false;
	}
	if (name == "player_spawn") {
		blackboard->reset();
		vision.reset();
		world->reset();
		area = nullptr;
		planner->resetPlanning(true);
		return true;
	}
	if (name == "player_death") {
		if (area != nullptr) {
			area->IncreaseDanger(getTeam(), mybotDangerAmt.GetFloat());
		}
		return false;
	}
	if (name == "player_hurt") {
		int attacker = event->getInt("attacker");
		if (event->getInt("attacker") == getUserId()) {
			return false;
		}
		world->updateState(WorldProp::HURT, true);
		for (auto player: Player::getPlayers()) {
			if (player.second->getUserId() == attacker) {
				wantToListen = false;
				viewTarget = player.second->getEyesPos();
				break;
			}
		}
		return true;
	}
	return false;
}

bool Bot::receive(edict_t* sender, const CCommand& command) {
	Player *player = getPlayer(sender);
	if (player != nullptr 
		&& player->getTeam() == getTeam()
			&& canSee(*player)
			&& voiceMessageSender.isMessage<AreaClearVoiceMessage>(command.Arg(0))) {
		world->updateState(WorldProp::HEARD_AREA_CLEAR, true);
	}
	return false;
}

void Bot::setWorld(World* world) {
	this->world = world;
}

const CBotCmd &Bot::getCmd() const {
	return blackboard->getCmd();
}

int Bot::getPlayerClass() const {
	return playerClassVar == nullptr ? -1 : playerClassVar->getPlayerClass();
}

void Bot::traceMove(CGameTrace &traceResult, const Vector &start,
		const Vector &goal, bool crouch, const ITraceFilter &filter) const {
	Vector mins = getEdict()->GetCollideable()->OBBMins(),
			maxs = getEdict()->GetCollideable()->OBBMaxs(),
			heading(goal - start);
	if (fabs(heading.x) > fabs(heading.y)) {
		(heading.x < 0.0f ? maxs.x : mins.x) = 0.0f;
	} else {
		(heading.y < 0.0f ? maxs.y : mins.y) = 0.0f;
	}
	if (crouch) {
		// magic number from https://developer.valvesoftware.com/wiki/Dimensions#Map_Grid_Units:_quick_reference
		// for some reason the OBBMaxs returns 60
		maxs.z -= 24.0f;
	}
	mins.z += 5.0f;
	extern ConVar mybot_debug;
	UTIL_TraceHull(heading.Normalized() * HalfHumanWidth + start,
			goal, mins, maxs, MASK_PLAYERSOLID, filter, &traceResult, mybot_debug.GetBool());
}

bool Bot::canShoot(trace_t& result, const Vector &vecAbsEnd) const {
	result.fraction = 0.0f;
	Vector start = getEyesPos();
	FilterSelfAndEnemies filter(getEdict());
	UTIL_TraceLine(start, vecAbsEnd, MASK_SHOT, &filter, &result);
	return !result.DidHit();
}

class VisionFilter: public CTraceFilter {
public:
	virtual ~VisionFilter() {
	}

	bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask ) override
	{
		std::string name(entityFromEntityHandle(pServerEntity)->GetClassName());
		return name.find("func_team") == name.npos && name.find("prop") == name.npos
				&& name.find("player") == name.npos;
	}
};

void Bot::canSee(trace_t& result, const Vector& start, const Vector& end) {
	VisionFilter filter;
	UTIL_TraceLine(start, end, MASK_ALL, &filter, &result);;
}

bool Bot::canSee(const Vector &vecAbsEnd) const {
	trace_t result;
	VisionFilter filter;
	canSee(result, getEyesPos(), vecAbsEnd);
	return !result.DidHit();
}

bool Bot::canSee(edict_t* target) const {
	return canSee(target->GetCollideable()->GetCollisionOrigin());
}

Vector Bot::getFacing() const {
	Vector facing;
	AngleVectors(getFacingAngle(), &facing);
	return facing;
}

void Bot::lookStraight() {
	this->viewTarget.z = getEyesPos().z;
}

float Bot::getAimAccuracy() const {
	return (viewTarget - getEyesPos()).Normalized().Dot(getFacing());
}

bool Bot::canShoot(const Vector &vecAbsEnd) const {
	trace_t result;
	return canShoot(result, vecAbsEnd);
}

void Bot::listen() {
	if (!wantToListen) {
		return;
	}
	float loudest = 0.0f;
	float closest = INFINITY;
	for (auto player: Player::getPlayers()) {
		if (player.second == this) {
			continue;
		}
		Vector position = player.second->getCurrentPosition();
		float noiseRange = player.second->getNoiseRange(),
				dist = getCurrentPosition().DistTo(position) < noiseRange;
		if ((isEnemy(*player.second) || dist > 300.0f)
				&& dist < noiseRange) {
			position.z += 31.0f; // center mass
			if ((noiseRange > loudest
					|| (noiseRange == loudest && dist < closest))
					&& canSee(*player.second)) {
				loudest = noiseRange;
				closest = dist;
				viewTarget = player.second->getEyesPos();
			}
		}
	}
}
