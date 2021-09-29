#include "Bot.h"

#include "Blackboard.h"
#include "Vision.h"
#include "World.h"
#include <event/EventInfo.h>
#include <goap/action/GoToAction.h>
#include <goap/GoalManager.h>
#include <move/Navigator.h>
#include <move/RotationManager.h>
#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>
#include <nav_mesh/nav_area.h>
#include <util/SimpleException.h>
#include <util/BasePlayer.h>
#include <util/UtilTrace.h>
#include <ivdebugoverlay.h>
#include <in_buttons.h>

PlayerClasses Bot::CLASSES = nullptr;

ConVar mybot_rot_speed("mybot_rot_speed", "0.5", 0,
		"determines rotational acceleration rate in degrees");
ConVar mybot_mimic("mybot_mimic", "0");

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
		if (isDead()) {
			despawn();
		} else if (getPlayerClass() != desiredClassId) {
			extern IServerPluginHelpers* helpers;
			helpers->ClientCommand(getEdict(), (*CLASSES)[getTeam() - 2][desiredClassId]);
		} else {
			if ((resetPlanner || world->think(*blackboard))
					&& !blackboard->isOnLadder()) {
				planner->resetPlanning(false);
				resetPlanner = false;
			}
			vision.updateVisiblity(*blackboard);
			if (inGame) {
				wantToListen = true;
				cmd.Reset();
				int best = arsenal.getBestWeapon(*blackboard,
						[] (const Weapon*, Blackboard&, float) {
					return false;
				});
				if (best != 0) {
					arsenal.setBestWeaponIdx(best);
				}
				int currentWeapon = arsenal.getCurrWeaponIdx();
				if (currentWeapon > 0
						&& arsenal.getWeapon(currentWeapon)->isOutOfAmmo(getEdict())) {
					world->updateState(WorldProp::USING_BEST_WEAP, false);
				}
				planner->execute();
				listen();
				extern ConVar mybot_debug;
				if (mybot_debug.GetBool()) {
					extern IVDebugOverlay *debugoverlay;
					debugoverlay->AddLineOverlay(getEyesPos(), blackboard->getViewTarget(), 0,
							255, 0, true,
							NDEBUG_PERSIST_TILL_NEXT_SERVER);
				}
				VectorAngles(blackboard->getViewTarget() - getEyesPos(), cmd.viewangles);
				rotation.getUpdatedPosition(cmd.viewangles, getFacingAngle(),
						mybot_rot_speed.GetFloat());
				if (cmd.weaponselect != 0) {
					world->updateState(WorldProp::USING_BEST_WEAP, true);
				}
			} else {
				blackboard->getButtons().tap(IN_ATTACK);
			}
		}
		cmd.buttons = blackboard->getButtons().getPressed();
		extern CGlobalVars *gpGlobals;
		cmd.tick_count = gpGlobals->tickcount;
		if (mybot_mimic.GetBool()) {
			auto& players = Player::getPlayers();
			cmd = players[players.Find(1)]->getInfo()->GetLastUserCommand();
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
	if (eventUserId == getUserId()) {
		if (name == "player_spawn") {
			despawn();
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
			despawn();
			return false;
		}
		if (name == "player_hurt") {
			int attacker = event->getInt("attacker");
			if (event->getInt("attacker") == getUserId()) {
				return false;
			}
			world->updateState(WorldProp::HURT, true);
			auto& players = Player::getPlayers();
			FOR_EACH_MAP_FAST(players, i) {
				if (players[i]->getUserId() == attacker) {
					wantToListen = false;
					blackboard->setViewTarget(players[i]->getEyesPos());
					blackboard->setTargetedPlayer(players[i]);
					break;
				}
			}
			return true;
		}
	} else if (name == "player_death" || name == "player_disconnect") {
		auto targeted = blackboard->getTargetedPlayer();
		extern IVEngineServer* engine;
		if (targeted != nullptr && eventUserId == targeted->getUserId()) {
			blackboard->setTargetedPlayer(nullptr);
		}
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


class FilterSelfAndEnemies: public CTraceFilter {
public:
	FilterSelfAndEnemies(edict_t* self,
			edict_t* target) : self(self), target(target) {
	}

	virtual ~FilterSelfAndEnemies() {
	}

	bool ShouldHitEntity(IHandleEntity *pHandleEntity, int contentsMask) {
		auto& players = Player::getPlayers();
		if (target != nullptr && pHandleEntity == target->GetIServerEntity()) {
			return true;
		}
		if (pHandleEntity == self->GetIServerEntity()) {
			return false;
		}
		FOR_EACH_MAP_FAST(players, i) {
			if (!players[i]->isDead()
					&& players[i]->getEdict()->GetIServerEntity()
							== pHandleEntity) {
				return false;
			}
		}
		return true;
	}

private:
	edict_t* self, *target;
};

bool Bot::canSee(trace_t& result, const Vector &vecAbsEnd, edict_t* target) const {
	if (target == nullptr) {
		return false;
	}
	result.fraction = 0.0f;
	Vector start = getEyesPos();
	FilterSelfAndTarget filter(getEdict()->GetIServerEntity(),
			target->GetIServerEntity());
	UTIL_TraceLine(start, vecAbsEnd, MASK_ALL, &filter, &result);
	return !result.DidHit();
}

bool Bot::canSee(const Vector &vecAbsEnd, edict_t* target) const {
	trace_t result;
	return canSee(result, vecAbsEnd, target);
}

void Bot::despawn() {
	planner->resetPlanning(true);
	blackboard->getButtons().tap(IN_ATTACK);
}

void Bot::listen() {
	if (!wantToListen) {
		return;
	}
	float loudest = 0.0f;
	float closest = INFINITY;
	auto& players = Player::getPlayers();
	FOR_EACH_MAP_FAST(players, i) {
		if (players[i] == this) {
			continue;
		}
		Vector position = players[i]->getCurrentPosition();
		float noiseRange = players[i]->getNoiseRange(),
				dist = getCurrentPosition().DistTo(position) < noiseRange;
		int team = getTeam();
		if ((team < 1 || team != players[i]->getTeam() || dist > 100.0f)
				&& dist < noiseRange) {
			position.z += 31.0f; // center mass
			if ((noiseRange > loudest
					|| (noiseRange == loudest && dist < closest))
					&& canSee(position, players[i]->getEdict())) {
				loudest = noiseRange;
				closest = dist;
				blackboard->setViewTarget(players[i]->getEyesPos());
			}
		}
	}
}
