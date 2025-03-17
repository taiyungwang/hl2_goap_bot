#include "Bot.h"

#include "Vision.h"
#include "World.h"
#include "FilterSelfAndEnemies.h"
#include <goap/action/GoToAction.h>
#include <goap/GoalManager.h>
#include <move/Navigator.h>
#include <move/RotationManager.h>
#include <weapon/Weapon.h>
#include <weapon/WeaponFunction.h>
#include <nav_mesh/nav_area.h>
#include <util/SimpleException.h>
#include <util/BasePlayer.h>
#include <util/BaseCombatWeapon.h>
#include <util/EntityUtils.h>
#include <util/UtilTrace.h>
#include <shareddefs.h>
#include <ivdebugoverlay.h>
#include <IEngineTrace.h>
#include <in_buttons.h>

PlayerClasses Bot::CLASSES = nullptr;

static ConVar mybot_mimic("mybot_mimic", "0");

extern IVEngineServer *engine;

Bot::Bot(edict_t* ent, const WeaponBuilders& arsenal,
	CommandHandler& commandHandler,
	const std::unordered_map<unsigned int, std::string> &messages) :
	Player(ent, arsenal), Receiver(commandHandler),
	messages(messages) {
	navigator = std::make_shared<Navigator>(this);
	listenForGameEvent({"player_hurt"});
}

Bot::~Bot() {
	delete planner;
	delete world;
	if (playerClassVar != nullptr) {
		delete playerClassVar;
	}
}

void Bot::think() {
	try {
		Player::think();
		if (!inGame) {
			buttons.tap(IN_ATTACK);
		} else if (getPlayerClass() != desiredClassId) {
			extern IServerPluginHelpers* helpers;
			helpers->ClientCommand(getEdict(), (*CLASSES)[getTeam() - 2][desiredClassId]);
		} else {
			vision.updateVisiblity(this);
			wantToListen = true;
			cmd.Reset();
			if ((world->think(this) || resetPlanner)
					&& !isOnLadder()) {
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
			if (vision.getTargetedPlayer() > 0) {
				cmd.viewangles.x += RandomFloat(-aimOffset, aimOffset);
				cmd.viewangles.y += RandomFloat(-aimOffset, aimOffset);
			}
			rotation.getUpdatedPosition(cmd.viewangles, getFacingAngle());
		}
		cmd.buttons = buttons.getPressed();
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

void Bot::FireGameEvent(IGameEvent* event) {
	Player::FireGameEvent(event);
	std::string name(event->GetName());
	int eventUserId = event->GetInt("userid");
	if (eventUserId != getUserId()) {
		return;
	}
	// bot owns this event.
	if (name == "player_spawn") {
		blocker = nullptr;
		vision.reset();
		world->reset();
		planner->resetPlanning(true);
	} else if (name == "player_hurt") {
		int attacker = event->GetInt("attacker");
		if (attacker == getUserId()) {
			return;
		}
		for (auto player: Player::getPlayers()) {
			if (player.second->getUserId() == attacker
					&& player.first != vision.getTargetedPlayer()) {
				wantToListen = false;
				world->updateState(WorldProp::HURT, true);
				viewTarget = player.second->getEyesPos();
				break;
			}
		}
	}
}

bool Bot::sendVoiceMessage(const unsigned int messageType) {
	if (messages.find(messageType) == messages.end()) {
		return false;
	}
	float time = engine->Time();
	while (!sentMessages.empty()
			&& time - std::get<1>(sentMessages.front()) > 10.0f) {
		sentMessages.pop_front();
	}
	const auto &message = messages.at(messageType);
	for (const auto &sent : sentMessages) {
		if (std::get<0>(sent) || message == std::get<2>(sent)) {
			return false;
		}
	}
	extern IServerPluginHelpers *helpers;
	helpers->ClientCommand(getEdict(), message.c_str());
	sentMessages.emplace_back(std::make_tuple(true, time, message));
	return true;
}

bool Bot::receive(edict_t* sender, const CCommand& command) {
	if ((hasRadio && Player::getPlayer(sender)->getTeam() == getTeam())
			|| vision.getNearbyTeammates().find(engine->IndexOfEdict(sender))
			!= vision.getNearbyTeammates().end()) {
		for (auto& sent: sentMessages) {
			if (std::get<2>(sent) == command.Arg(0)) {
				std::get<1>(sent) = engine->Time();
				return false;
			}
		}
		sentMessages.emplace_back(std::make_tuple(false,
				engine->Time(), command.Arg(0)));
	}
	return false;
}

void Bot::setWorld(World* world) {
	this->world = world;
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

bool Bot::isOnLadder() const {
	return playerClassVar->isOnLadder();
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

void Bot::consoleMsg(const std::string& message) const {
	Msg("%s: %s\n", getName(), message.c_str());
}

void Bot::consoleWarn(const std::string& message) const {
	Warning("%s: %s\n", getName(), message.c_str());
}

int Bot::getBestWeapon() const {
	int targetedPlayer = vision.getTargetedPlayer();
	edict_t* target = nullptr;
	if (targetedPlayer > 0) {
		target = Player::getPlayer(targetedPlayer)->getEdict();
	} else if (blocker != nullptr) {
		target = blocker;
	}
	return Player::getBestWeapon(target);
}
