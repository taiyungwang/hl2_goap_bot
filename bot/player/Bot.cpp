#include "Bot.h"

#include "Blackboard.h"
#include "Vision.h"
#include "World.h"
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

static ConVar mybot_rot_speed("mybot_rot_speed", "0.15", 0,
		"determines rotational acceleration rate in degrees");

ConVar mybot_mimic("mybot_mimic", "0");

extern IVEngineServer* engine;
extern CGlobalVars *gpGlobals;

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
			if ((resetPlanner || world->think(*blackboard))
					&& !blackboard->isOnLadder()) {
				planner->resetPlanning(false);
				resetPlanner = false;
			}
			vision.updateVisiblity(this);
			wantToListen = true;
			cmd.Reset();
			int best = arsenal->getBestWeapon(*blackboard,
					[] (const Weapon*, Blackboard&, float) {
				return false;
			});
			if (best != 0) {
				arsenal->setBestWeaponIdx(best);
			}
			int currentWeapon = arsenal->getCurrWeaponIdx();
			if (currentWeapon > 0
					&& arsenal->getWeapon(currentWeapon)->isOutOfAmmo(getEdict())) {
				world->updateState(WorldProp::USING_BEST_WEAP, false);
			}
			planner->execute();
			listen();
			extern ConVar mybot_debug;
			if (mybot_debug.GetBool()) {
				extern IVDebugOverlay *debugoverlay;
				debugoverlay->AddLineOverlay(getEyesPos(), viewTarget, 0,
						255, 0, true,
						NDEBUG_PERSIST_TILL_NEXT_SERVER);
			}
			VectorAngles(viewTarget - getEyesPos(), cmd.viewangles);
			rotation.getUpdatedPosition(cmd.viewangles, getFacingAngle(),
					mybot_rot_speed.GetFloat());
			if (cmd.weaponselect != 0) {
				world->updateState(WorldProp::USING_BEST_WEAP, true);
			}
		}
		cmd.buttons = blackboard->getButtons().getPressed();
		cmd.tick_count = gpGlobals->tickcount;
		if (mybot_mimic.GetBool()) {
			auto& players = Player::getPlayers();
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
	CUtlString name(event->getName());
	int eventUserId = event->getInt("userid");
	// bot owns this event.
	if (eventUserId != getUserId()) {
		return false;
	}
	if (name == "player_spawn") {
		CNavArea* area = blackboard->getNavigator()->getLastArea();
		if (area != nullptr) {
			area->IncreaseDanger(getTeam(), 1.0f);
		}
		resetPlanner = true;
		blackboard->reset();
		world->reset();
		return true;
	}
	if (name == "player_death") {
		planner->resetPlanning(true);
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
	if (player->getTeam() == getTeam()
			&& canSee(*player)
			&& voiceMessageSender.isMessage<AreaClearVoiceMessage>(command.Arg(0))) {
		world->updateState(WorldProp::HEARD_AREA_CLEAR, true);
	}
	return false;
}

void Bot::setWorld(World* world) {
	this->world = world;
}

CBotCmd* Bot::getCmd() const {
	return blackboard == nullptr ? nullptr : &blackboard->getCmd();
}

int Bot::getPlayerClass() const {
	return playerClassVar == nullptr ? -1 : playerClassVar->getPlayerClass();
}

class FilterSelfAndEnemies: public CTraceFilterEntitiesOnly {
public:
	FilterSelfAndEnemies(edict_t* self) : self(self) {
	}

	virtual ~FilterSelfAndEnemies() {
	}

	bool ShouldHitEntity(IHandleEntity *pHandleEntity, int contentsMask) override {
		if (pHandleEntity == self->GetIServerEntity()) {
			return false;
		}
		int idx = engine->IndexOfEdict(entityFromEntityHandle(const_cast<const IHandleEntity*>(pHandleEntity)));
		return (idx < 1 || idx > gpGlobals->maxClients)
				|| !Player::getPlayer(idx)->isEnemy(*Player::getPlayer(self));
	}

private:
	edict_t *self;
};

bool Bot::canShoot(trace_t& result, const Vector &vecAbsEnd, edict_t* target) const {
	if (target == nullptr) {
		return false;
	}
	result.fraction = 0.0f;
	Vector start = getEyesPos();
	FilterSelfAndEnemies filter(getEdict());
	UTIL_TraceLine(start, vecAbsEnd, MASK_ALL, &filter, &result);
	return !result.DidHit();
}

class VisionFilter: public CTraceFilter {
public:
	virtual ~VisionFilter() {
	}

	bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask ) override
	{
		return FClassnameIs(entityFromEntityHandle(pServerEntity), "worldspawn");
	}
};


bool Bot::canSee(const Player& player) const {
	trace_t result;
	VisionFilter filter;
	UTIL_TraceLine(getEyesPos(), player.getEyesPos(),
			MASK_ALL, &filter, &result);
	return !result.DidHit();
}

bool Bot::canSee(edict_t* target) const {
	trace_t result;
	VisionFilter filter;
	UTIL_TraceLine(getEyesPos(), target->GetCollideable()->GetCollisionOrigin(),
			MASK_ALL, &filter, &result);
	return !result.DidHit();
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

bool Bot::canShoot(const Vector &vecAbsEnd, edict_t* target) const {
	trace_t result;
	return canShoot(result, vecAbsEnd, target);
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
